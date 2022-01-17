#include "exrpc.hpp"
#include "exrpc_expose.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <algorithm>
#include <string.h>

#ifndef NO_PROGRAM_OPTION
#include <boost/program_options.hpp>
#endif
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

namespace frovedis {

#define MAX_CONNECTION 65536
static int crnt_connection = 0;
static pthread_mutex_t send_lock[MAX_CONNECTION] = {PTHREAD_MUTEX_INITIALIZER};
static pthread_mutex_t send_management_lock = PTHREAD_MUTEX_INITIALIZER;

static int watch_sockfd;
static std::unordered_map<exrpc_node, int> send_connection_pool;
// need to be exposed to exrpc_result and send_exrpcreq_oneway
std::unordered_map<int, pthread_mutex_t*> send_connection_lock;
  
static std::vector<int> recv_connection_pool;
static int listen_port_pool = 0;
static int listen_fd_pool = -1;

bool operator==(const exrpc_node& lhs, const exrpc_node& rhs) {
  return lhs.hostname == rhs.hostname && lhs.rpcport == rhs.rpcport;
}

void mywrite(int fd, const char* write_data, size_t to_write) {
  while(to_write > 0) {
    ssize_t write_count = ::write(fd, write_data, to_write);
    if(write_count < 0) {
      throw std::runtime_error(std::string("error in mywrite: ") +
                               strerror(errno));
    } else {
      to_write -= write_count;
      write_data += write_count;
    }
  }
}

void myread(int fd, char* read_data, size_t to_read) {
  while(to_read > 0) {
    ssize_t read_count = ::read(fd, read_data, to_read);
    if(read_count < 0) {
      throw std::runtime_error(std::string("error in myread: ") +
                               strerror(errno));
    } else if (read_count == 0) {
      throw std::runtime_error(std::string("peer connection closed"));
    } else {
      to_read -= read_count;
      read_data += read_count;
    }
  }
}

int handle_exrpc_listen(int& port) {
  int sockfd;
  struct sockaddr_in reader_addr; 

  if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    throw std::runtime_error
      (std::string("handle_exrpc_listen: error in creating socket: ") +
       strerror(errno));
  }
  int yes = 1;
  setsockopt(sockfd,
             SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
  bzero(&reader_addr, sizeof(reader_addr));
  reader_addr.sin_family = PF_INET;
  reader_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  reader_addr.sin_port = htons(port); // allow to be zero
  if(::bind(sockfd, (struct sockaddr *)&reader_addr,
            sizeof(reader_addr)) < 0) {
    throw std::runtime_error
      (std::string("handle_exrpc_listen: error in bind: ") + strerror(errno));
  }
  if(port == 0) {
    bzero(&reader_addr, sizeof(reader_addr));
    socklen_t reader_len = sizeof(reader_addr);
    if(::getsockname(sockfd, (struct sockaddr *)&reader_addr,
                     &reader_len) < 0) {
      throw std::runtime_error
        (std::string("handle_exrpc_listen: error in getsockname: ") +
         strerror(errno));
    }
    port = ntohs(reader_addr.sin_port); // ephemeral port
  }
  if(listen(sockfd, 128) < 0) {
    ::close(sockfd);
    throw std::runtime_error
      (std::string("handle_exrpc_listen: error in listen: ") +
       strerror(errno));
  }
  return sockfd;
}

int handle_exrpc_listen_pooled(int& port) {
  if(listen_port_pool != 0) {
    port = listen_port_pool;
    return listen_fd_pool;
  } else {
    listen_fd_pool = handle_exrpc_listen(port);
    listen_port_pool = port;
    return listen_fd_pool;
  }
}

bool handle_exrpc_accept_by_client(int sockfd, int timeout, int& new_sockfd) {
  struct sockaddr_in writer_addr;
  socklen_t writer_len = sizeof(writer_addr);
  if(timeout != 0) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    int retval = select(sockfd+1, &rfds, NULL, NULL, &tv);
    if(retval == -1)
      throw std::runtime_error
        (std::string("handle_exrpc_accept: error in select: ") +
         strerror(errno));
    else if(retval == 0) return false; // timeout
  } 
  if((new_sockfd = ::accept(sockfd, (struct sockaddr *)&writer_addr,
                            &writer_len)) < 0) {
    ::close(sockfd);
    throw std::runtime_error
      (std::string("handle_exrpc_accept: error in accept: ") + strerror(errno));
  }
  return true;
}

bool handle_exrpc_accept_pooled(int sockfd, int& new_sockfd, bool from_driver) {
  struct sockaddr_in writer_addr;
  socklen_t writer_len = sizeof(writer_addr);
  if(from_driver) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(watch_sockfd, &rfds);
    FD_SET(sockfd, &rfds);
    int maxfd = std::max(watch_sockfd, sockfd);
    for(size_t i = 0; i < recv_connection_pool.size(); i++) {
      auto crntfd = recv_connection_pool[i];
      FD_SET(crntfd, &rfds);
      if(crntfd > maxfd) maxfd = crntfd;
    }
    int retval;
    retval = select(maxfd+1, &rfds, NULL, NULL, NULL);
    if(retval == -1) {
      throw std::runtime_error
        (std::string("handle_exrpc_accept: error in select: ") +
         strerror(errno));
    } else if(FD_ISSET(watch_sockfd, &rfds)) {
      return false; // connection to client is closed
    } else if(FD_ISSET(sockfd, &rfds)) {
      if((new_sockfd = ::accept(sockfd, (struct sockaddr *)&writer_addr,
                                &writer_len)) < 0) {
        ::close(sockfd);
        throw std::runtime_error
          (std::string("handle_exrpc_accept: error in accept: ") +
           strerror(errno));
      }
      int one = 1;
      if(setsockopt(new_sockfd, SOL_TCP, TCP_NODELAY, &one, sizeof(one))
         != 0) {
        throw std::runtime_error
          (std::string("handle_exrpc_accept_pooled: error in setsockopt: ") +
           strerror(errno));
      }
      recv_connection_pool.push_back(new_sockfd);
      return true;
    } else {
      for(size_t i = 0; i < recv_connection_pool.size(); i++) {
        auto crntfd = recv_connection_pool[i];
        if(FD_ISSET(crntfd, &rfds)) {
          new_sockfd = crntfd;
          return true;
        }
      }
      return false; // should not happen
    }
  } else { // worker; client availability is not checked
    if(recv_connection_pool.size() == 0) { // pool is empty
      if((new_sockfd = ::accept(sockfd, (struct sockaddr *)&writer_addr,
                                &writer_len)) < 0) {
        ::close(sockfd);
        throw std::runtime_error
          (std::string("handle_exrpc_accept: error in accept: ") +
           strerror(errno));
      }
      recv_connection_pool.push_back(new_sockfd);
      return true;
    } else { // check if pooled connection is used with select
      fd_set rfds;
      FD_ZERO(&rfds);
      FD_SET(sockfd, &rfds); // sockfd should be same as listen_fd_pool
      int maxfd = sockfd;
      for(size_t i = 0; i < recv_connection_pool.size(); i++) {
        auto crntfd = recv_connection_pool[i];
        FD_SET(crntfd, &rfds);
        if(crntfd > maxfd) maxfd = crntfd;
      }
      int retval;
      retval = select(maxfd+1, &rfds, NULL, NULL, NULL);
      if(retval == -1) {
        throw std::runtime_error
          (std::string("handle_exrpc_accept: error in select: ") +
           strerror(errno));
      } else if(FD_ISSET(sockfd, &rfds)) {
        if((new_sockfd = ::accept(sockfd, (struct sockaddr *)&writer_addr,
                                  &writer_len)) < 0) {
          ::close(sockfd);
          throw std::runtime_error
            (std::string("handle_exrpc_accept: error in accept: ") +
             strerror(errno));
        }
        int one = 1;
        if(setsockopt(new_sockfd, SOL_TCP, TCP_NODELAY, &one, sizeof(one))
           != 0) {
          throw std::runtime_error
            (std::string("handle_exrpc_accept_pooled: error in setsockopt: ") +
             strerror(errno));
        }
        recv_connection_pool.push_back(new_sockfd);
        return true;
      } else {
        for(size_t i = 0; i < recv_connection_pool.size(); i++) {
          auto crntfd = recv_connection_pool[i];
          if(FD_ISSET(crntfd, &rfds)) {
            new_sockfd = crntfd;
            return true;
          }
        }
        return false; // should not happen
      }
    }
  }
}

int handle_exrpc_connect(const std::string& hostname, int rpcport) {
  int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) {
    throw std::runtime_error(
      std::string("handle_exrpc_connect: error in creating socket: ") +
      strerror(errno));
  }
  struct addrinfo hints, *res;
  int err;
  bzero(&hints, sizeof(hints));
  hints.ai_flags = AI_NUMERICSERV;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;
  auto portstr = boost::lexical_cast<std::string>(rpcport);
  if ((err = getaddrinfo(hostname.c_str(), portstr.c_str(), &hints, &res))
      != 0) {
    throw
      std::runtime_error
      (std::string("handle_exrpc_connect: error in getaddrinfo: ") +
       gai_strerror(err));
  }
  bool connected = false;
  for (; res != NULL; res = res->ai_next) {
    if(connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
      continue;
    } else {
      connected = true;
      break;
    }
  }
  freeaddrinfo(res);
  if(!connected) {
    throw std::runtime_error
      (std::string("handle_exrpc_connect: error in connect: ") +
       strerror(errno));
  }
  int one = 1;
  if(setsockopt(sockfd, SOL_TCP, TCP_NODELAY, &one, sizeof(one)) != 0) {
    throw std::runtime_error
      (std::string("handle_exrpc_connect: error in setsockopt: ") +
       strerror(errno));
  }
  return sockfd;
}

int handle_exrpc_connect_pooled(const std::string& hostname, int rpcport) {
  pthread_mutex_lock(&send_management_lock);
  auto n = exrpc_node(hostname, rpcport);
  auto it = send_connection_pool.find(n);
  if(it != send_connection_pool.end()) {
    pthread_mutex_unlock(&send_management_lock);
    auto fd = it->second;
    auto it2 = send_connection_lock.find(fd);
    if(it2 == send_connection_lock.end()) {
      throw std::runtime_error("internal error in handle_exrpc_connect_pooled");
    } else {
      pthread_mutex_lock(it2->second);
      return it->second;
    }
  } else {
    auto fd = handle_exrpc_connect(hostname, rpcport);
    pthread_mutex_t* mylock = send_lock + crnt_connection;
    send_connection_lock.insert(std::make_pair(fd, mylock));
    crnt_connection++;
    send_connection_pool.insert(std::make_pair(n, fd));
    pthread_mutex_unlock(&send_management_lock);
    if(crnt_connection > MAX_CONNECTION)
      throw std::runtime_error
        ("handle_exrpc_connect_pooled: too many connections");
    pthread_mutex_lock(mylock);
    return fd;
  }
  // connection locks are unlocked at exrpc_oneway.cc, exrpc_oneway_noexcept.cc,
  // exrpc_result.hpp, and send_exrpc_finish function.
}

int send_exrpcreq(exrpc_type type, exrpc_node& n, const std::string& funcname,
                  const std::string& serialized_arg) {
  int sockfd = handle_exrpc_connect_pooled(n.hostname, n.rpcport);
  exrpc_header hdr;
  hdr.type = type;
  hdr.funcname = funcname;
  hdr.arg_count = serialized_arg.size();
  
  my_portable_ostream result;
  my_portable_oarchive outar(result);
  outar & hdr;
  PORTABLE_OSTREAM_TO_STRING(result, serialized_hdr);
  uint32_t hdr_size = serialized_hdr.size();
  uint32_t hdr_size_nw = htonl(hdr_size);
  if(serialized_arg.size() < 65536) {
    size_t send_size = sizeof(hdr_size_nw) + hdr_size + serialized_arg.size();
    std::vector<char> buffer(send_size);
    auto bufferp = buffer.data();
    memcpy(bufferp, reinterpret_cast<char*>(&hdr_size_nw), sizeof(hdr_size_nw));
    bufferp += sizeof(hdr_size_nw);
    memcpy(bufferp, serialized_hdr.c_str(), hdr_size);
    bufferp += hdr_size;
    memcpy(bufferp, serialized_arg.c_str(), serialized_arg.size());
    mywrite(sockfd, buffer.data(), send_size);
  } else {
    mywrite(sockfd, reinterpret_cast<char*>(&hdr_size_nw), sizeof(hdr_size_nw));
    mywrite(sockfd, serialized_hdr.c_str(), hdr_size);
    mywrite(sockfd, serialized_arg.c_str(), serialized_arg.size());
  }
  return sockfd;
}

void send_exrpc_finish(exrpc_node& n) {
  auto sockfd = send_exrpcreq(exrpc_type::exrpc_finalize_type, n,
                              std::string(""), std::string(""));
  auto it = send_connection_lock.find(sockfd);
  if(it == send_connection_lock.end()) {
    throw std::runtime_error("internal error in send_exrpc_finish");
  } else {
    pthread_mutex_unlock(it->second);
  }
}

void inform_no_exposed_function(int fd, const std::string& funcname) {
  std::string what = std::string("not exposed function is called: ") + funcname;
  char exception_caught = true;
  mywrite(fd, &exception_caught, 1);
  exrpc_count_t send_data_size = what.size();
  exrpc_count_t send_data_size_nw = myhtonll(send_data_size);
  mywrite(fd, reinterpret_cast<char*>(&send_data_size_nw),
          sizeof(send_data_size_nw));
  mywrite(fd, what.c_str(), send_data_size);
}

bool handle_exrpc_process(int new_sockfd) {
  uint32_t hdr_size_nw;
  myread(new_sockfd, reinterpret_cast<char*>(&hdr_size_nw),
         sizeof(hdr_size_nw));
  uint32_t hdr_size = ntohl(hdr_size_nw);
  std::string serialized_hdr;
  serialized_hdr.resize(hdr_size);
  myread(new_sockfd, const_cast<char*>(serialized_hdr.c_str()), hdr_size);
  exrpc_header hdr;
  STRING_TO_PORTABLE_ISTREAM(hdrss, serialized_hdr);
  my_portable_iarchive hdrar(hdrss);
  hdrar & hdr;
  std::string funcname = hdr.funcname;
  if(hdr.type == exrpc_type::exrpc_async_type) {
    std::string serialized_arg;
    serialized_arg.resize(hdr.arg_count);
    myread(new_sockfd, &serialized_arg[0], hdr.arg_count);
    typedef void(*wptype)(intptr_t, my_portable_iarchive&,
                          my_portable_oarchive&);
    if(expose_table.find(funcname) == expose_table.end()) {
      inform_no_exposed_function(new_sockfd, funcname);
      return true;
    } else {
      wptype wpt = reinterpret_cast<wptype>(expose_table[funcname].first);
      STRING_TO_PORTABLE_ISTREAM(inss, serialized_arg);
      my_portable_iarchive inar(inss);
      my_portable_ostream result;
      my_portable_oarchive outar(result);
      std::string what;
      char exception_caught = false; // 1 byte to avoid endian conv.
      try {
        wpt(expose_table[funcname].second, inar, outar);
      } catch (std::exception& e) {
        exception_caught = true;
        what = e.what();
      }
      std::string resultstr;
      if(!exception_caught) {
        PORTABLE_OSTREAM_TO_STRING(result, tmp);
        resultstr = std::move(tmp);
      }
      else resultstr = what;
      exrpc_count_t send_data_size = resultstr.size();
      exrpc_count_t send_data_size_nw = myhtonll(send_data_size);
      if(resultstr.size() < 65536) {
        size_t send_size = 1 + sizeof(send_data_size_nw) + resultstr.size();
        std::vector<char> buffer(send_size);
        auto bufferp = buffer.data();
        memcpy(bufferp, &exception_caught, 1);
        bufferp += 1;
        memcpy(bufferp, reinterpret_cast<char*>(&send_data_size_nw),
               sizeof(send_data_size_nw));
        bufferp += sizeof(send_data_size_nw);
        memcpy(bufferp, resultstr.c_str(), resultstr.size());
        mywrite(new_sockfd, buffer.data(), send_size);
      } else {
        mywrite(new_sockfd, &exception_caught, 1); // should be 1
        mywrite(new_sockfd, reinterpret_cast<char*>(&send_data_size_nw),
                sizeof(send_data_size_nw));
        mywrite(new_sockfd, resultstr.c_str(), resultstr.size());
      }
      return true;
    }
  } else if(hdr.type == exrpc_type::exrpc_oneway_type) {
    std::string serialized_arg;
    serialized_arg.resize(hdr.arg_count);
    myread(new_sockfd, &serialized_arg[0], hdr.arg_count);
    typedef void(*wptype)(intptr_t, my_portable_iarchive&);
    if(expose_table.find(funcname) == expose_table.end()) {
      inform_no_exposed_function(new_sockfd, funcname);
      return true;
    } else {
      wptype wpt = reinterpret_cast<wptype>(expose_table[funcname].first);
      STRING_TO_PORTABLE_ISTREAM(inss, serialized_arg);
      my_portable_iarchive inar(inss);
      my_portable_ostream result;
      my_portable_oarchive outar(result);
      std::string what;
      char exception_caught = false; // 1 byte to avoid endian conv.
      try {
        wpt(expose_table[funcname].second, inar);
      } catch (std::exception& e) {
        exception_caught = true;
        what = e.what();
      }
      mywrite(new_sockfd, &exception_caught, 1); // should be 1
      if(exception_caught) {
        auto resultstr = what;
        exrpc_count_t send_data_size = resultstr.size();
        exrpc_count_t send_data_size_nw = myhtonll(send_data_size);
        mywrite(new_sockfd, reinterpret_cast<char*>(&send_data_size_nw),
                sizeof(send_data_size_nw));
        mywrite(new_sockfd, resultstr.c_str(), resultstr.size());
      }
      return true;
    }
  } else if(hdr.type == exrpc_type::exrpc_oneway_noexcept_type) {
    std::string serialized_arg;
    serialized_arg.resize(hdr.arg_count);
    myread(new_sockfd, &serialized_arg[0], hdr.arg_count);
    typedef void(*wptype)(intptr_t, my_portable_iarchive&);
    if(expose_table.find(funcname) == expose_table.end()) {
      inform_no_exposed_function(new_sockfd, funcname);
      return true;
    } else {
      wptype wpt = reinterpret_cast<wptype>(expose_table[funcname].first);
      STRING_TO_PORTABLE_ISTREAM(inss, serialized_arg);
      my_portable_iarchive inar(inss);
      my_portable_ostream result;
      my_portable_oarchive outar(result);
      wpt(expose_table[funcname].second, inar);
      return true;
    }
  } else if(hdr.type == exrpc_type::exrpc_rawsend_type) {
    exrpc_ptr_t writeptr;
    myread(new_sockfd, reinterpret_cast<char*>(&writeptr), sizeof(exrpc_ptr_t));
    myread(new_sockfd, reinterpret_cast<char*>(writeptr), hdr.arg_count);
    return true;
  } else if(hdr.type == exrpc_type::exrpc_rawrecv_type) {
    // hdr.arg_count is reused as server->client size (not client->server size)
    exrpc_ptr_t readptr;
    myread(new_sockfd, reinterpret_cast<char*>(&readptr), sizeof(exrpc_ptr_t));
    mywrite(new_sockfd, reinterpret_cast<char*>(readptr), hdr.arg_count);
    return true;
  } else {
    // TODO: close pooled socket of workers?
    return false;
  }
}

bool handle_exrpc_onereq(int sockfd, bool from_driver) {
  int new_sockfd;
  auto r = handle_exrpc_accept_pooled(sockfd, new_sockfd, from_driver);
  if(r) r = handle_exrpc_process(new_sockfd);
  return r;
}

std::string get_server_name() {
  char server_hostname[1024];
  if(gethostname(server_hostname, 1024) == -1) {
    throw std::runtime_error
      (std::string("get_server_name: error in gethostname: ") +
       + strerror(errno));
  }
  return std::string(server_hostname);
}

exrpc_node invoke_frovedis_server(const std::string& command) {
  int port = 0;
  int sockfd = handle_exrpc_listen(port);
  char hostname[1024];
  if(gethostname(hostname, 1024) == -1) {
    throw std::runtime_error
      (std::string("invoke_frovedis_server: error in gethostname: ") +
       + strerror(errno));
  }
#ifdef USE_IP_EXRPC
  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;
  bzero(&hints, sizeof(hints));
  hints.ai_flags = AI_NUMERICSERV;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;
  if((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
    throw
      std::runtime_error(std::string("invoke_frovedis_server: error in getaddrinfo: ") +
                         gai_strerror(err));
  }
  addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;

  // assume that sizeof(intptr_t) is the same between server and client
  std::string total_command = command +
    " -h " + inet_ntoa(addr) + " -p " + boost::lexical_cast<std::string>(port) + " &";
  freeaddrinfo(res);
#else
  // assume that sizeof(intptr_t) is the same between server and client
  std::string total_command = command +
    " -h " + hostname + " -p " + boost::lexical_cast<std::string>(port) + " &";
#endif
  pid_t pid;
  if((pid = fork()) < 0) {
    throw std::runtime_error
      (std::string("invoke_frovedis_server: error in fork: ") +
       + strerror(errno));
  } else if(pid == 0) { // child process
    std::vector<std::string> args;
    boost::algorithm::split(args, total_command, boost::is_any_of(" "));
    auto args_size = args.size();
    std::vector<char*> argv(args_size+1);
    for(size_t i = 0; i < args_size; i++) {
      argv[i] = const_cast<char*>(args[i].c_str());
    }
    execvp(argv[0], argv.data());
    return exrpc_node(); // for disabling warning
  } else {
    int new_sockfd;
    bool is_ok = handle_exrpc_accept_by_client(sockfd, 5, new_sockfd);
    ::close(sockfd);
    if(is_ok) {
      int port;
      size_t server_name_size;
      std::string server_name;
      myread(new_sockfd, (char*)&port, sizeof(port));
      myread(new_sockfd, (char*)&server_name_size, sizeof(server_name_size));
      server_name.resize(server_name_size);
      char* server_namep = const_cast<char*>(server_name.c_str());
      myread(new_sockfd, server_namep, server_name_size);
      exrpc_node server_node;
      server_node.hostname = server_name;
      server_node.rpcport = port;
      char ack = 1;
      mywrite(new_sockfd, &ack, 1);
      return server_node;
      // new_sockfd is left untouched; will send RST when client aborts
    } else {
      auto kill_cmd = std::string("kill -s INT ") + std::to_string(pid);
      ::system(kill_cmd.c_str());
      throw std::runtime_error(std::string("invoke_frovedis_server: timeout.\n 1) check if the server invocation command is correct.\n 2) check if the hostname is in /etc/hosts or DNS.\n 3) confirm other processes are not running on the same VE."));
    }
  }
}

void init_frovedis_server(int argc, char* argv[]) {
  expose(prepare_parallel_exrpc_server);
  expose(get_parallel_exrpc_nodes_server);
  expose(wait_parallel_exrpc_server);
  expose(wait_parallel_exrpc_multi_server);
  flush_exposed(); // send expose information to workers
#ifdef NO_PROGRAM_OPTION
  std::string client_hostname;
  int client_port = 0;
  for(int i = 0; i < argc; i++) {
    if(std::string(argv[i]) == std::string("-h")) {
      client_hostname = std::string(argv[i+1]);
      i++;
    } else if(std::string(argv[i]) == std::string("-p")) {
      client_port = boost::lexical_cast<int>(std::string(argv[i+1]));
      i++;
    }
  }
  if(client_hostname == "" || client_port == 0)
    throw std::runtime_error("server: error in parsing arguments");
#else
  using namespace boost::program_options;

  options_description opt("option");
  opt.add_options()
    ("h,h", value<std::string>(), "hostname")
    ("p,p", value<int>(), "port number");

  variables_map argmap;
  store(command_line_parser(argc,argv).options(opt).allow_unregistered().
        run(), argmap);
  notify(argmap);

  std::string client_hostname;
  int client_port;

  if(argmap.count("h")){
    client_hostname = argmap["h"].as<std::string>();
  } else {
    std::cerr << "hostname is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }

  if(argmap.count("p")){
    client_port = argmap["p"].as<int>();
  } else {
    std::cerr << "port number is not specified" << std::endl;
    std::cerr << opt << std::endl;
    exit(1);
  }
#endif
  int accept_sockfd;
  int port = 0;
  try {
    // watch_sockfd is static variable
    watch_sockfd = handle_exrpc_connect(client_hostname, client_port);
    accept_sockfd = handle_exrpc_listen(port);
    auto server_name = get_server_name();
    auto server_name_size = server_name.size();
    mywrite(watch_sockfd, (char*)&port, sizeof(port));
    mywrite(watch_sockfd, (char*)&server_name_size, sizeof(server_name_size));
    mywrite(watch_sockfd, server_name.c_str(), server_name_size);
    // get ack from client
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(watch_sockfd, &rfds);
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    int retval = select(watch_sockfd+1, &rfds, NULL, NULL, &tv);
    if(retval == -1) {
      throw std::runtime_error
        (std::string("error in select: ") + strerror(errno));
    } else if(retval == 0) {
      throw std::runtime_error("client is not responding");
    }
    char ack;
    myread(watch_sockfd, &ack, 1);
  } catch (std::exception& e) {
    std::cerr << "error connection from server to client: "
              << e.what() << std::endl;
    exit(1);
  }
  while(handle_exrpc_onereq(accept_sockfd, true));
  ::close(accept_sockfd);
}

void finalize_frovedis_server(exrpc_node& n){
   send_exrpc_finish(n);
}

void prepare_parallel_exrpc_worker(exrpc_info& i) {
  int port = 0;
  i.sockfd = handle_exrpc_listen_pooled(port);
  char hostname[1024];
  if(gethostname(hostname, 1024) == -1) {
    throw std::runtime_error
      (std::string("prepare_parallel_exrpc_worker: error in gethostname: ") +
       + strerror(errno));
  }
#ifdef USE_IP_EXRPC
  struct addrinfo hints, *res;
  struct in_addr addr;
  int err;
  bzero(&hints, sizeof(hints));
  hints.ai_flags = AI_NUMERICSERV;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;
  if((err = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
    throw std::runtime_error
      (std::string("prepare_parallel_exrpc_worker: error in getaddrinfo: ") +
       gai_strerror(err));
  }
  addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
  i.node = exrpc_node(std::string(inet_ntoa(addr)), port);
  freeaddrinfo(res);
#else
  i.node = exrpc_node(std::string(hostname), port);
#endif
}

exptr<node_local<exrpc_info>> prepare_parallel_exrpc_server() {
  node_local<exrpc_info>* info = 
    new node_local<exrpc_info>(make_node_local_allocate<exrpc_info>());
  info->mapv(prepare_parallel_exrpc_worker);
  return make_exptr(info);
}

exptr<node_local<exrpc_info>> prepare_parallel_exrpc(exrpc_node& n) {
  return exrpc_async0(n, prepare_parallel_exrpc_server).get();
}

exrpc_node get_parallel_exrpc_nodes_server_helper(exrpc_info& i) {
  return i.node;
}

std::vector<exrpc_node>
get_parallel_exrpc_nodes_server(exptr<node_local<exrpc_info>>& info) {
  return info.to_ptr()->map(get_parallel_exrpc_nodes_server_helper).gather();
}

std::vector<exrpc_node>
get_parallel_exrpc_nodes(exrpc_node& n, exptr<node_local<exrpc_info>>& info) {
  return exrpc_async(n, get_parallel_exrpc_nodes_server, info).get();
}

void wait_parallel_exrpc_server_helper(exrpc_info& i) {
  handle_exrpc_onereq(i.sockfd, false);
}

void wait_parallel_exrpc_server(exptr<node_local<exrpc_info>>& info) {
  info.to_ptr()->mapv(wait_parallel_exrpc_server_helper);
}

void wait_parallel_exrpc(exrpc_node& n, exptr<node_local<exrpc_info>>& info) {
  // Use noexcept because this should not block!
  exrpc_oneway_noexcept(n, wait_parallel_exrpc_server, info);
}

void wait_parallel_exrpc_multi_server_helper(exrpc_info& i, size_t num_rpc) {
  for(size_t j = 0; j < num_rpc; j++) {
    handle_exrpc_onereq(i.sockfd, false);
  }
}

void wait_parallel_exrpc_multi_server(exptr<node_local<exrpc_info>>& info,
                                      std::vector<size_t>& num_rpc) {
  auto nl_num_rpc = make_node_local_scatter(num_rpc);
  info.to_ptr()->mapv(wait_parallel_exrpc_multi_server_helper, nl_num_rpc);
}

void wait_parallel_exrpc_multi(exrpc_node& n,
                               exptr<node_local<exrpc_info>>& info,
                               std::vector<size_t>& num_rpc) {
  // Use noexcept because this should not block!
  exrpc_oneway_noexcept(n, wait_parallel_exrpc_multi_server, info, num_rpc);
}

void exrpc_rawsend(exrpc_node& n, char* src,
                   exrpc_ptr_t dst, exrpc_count_t size) {
  int sockfd = handle_exrpc_connect_pooled(n.hostname, n.rpcport);
  exrpc_header hdr;
  hdr.type = exrpc_type::exrpc_rawsend_type;
  hdr.funcname = std::string();
  hdr.arg_count = size;
  
  my_portable_ostream result;
  my_portable_oarchive outar(result);
  outar & hdr;
  PORTABLE_OSTREAM_TO_STRING(result, serialized_hdr);
  uint32_t hdr_size = serialized_hdr.size();
  uint32_t hdr_size_nw = htonl(hdr_size);
  mywrite(sockfd, reinterpret_cast<char*>(&hdr_size_nw), sizeof(hdr_size_nw));
  mywrite(sockfd, serialized_hdr.c_str(), hdr_size);
  mywrite(sockfd, reinterpret_cast<char*>(&dst), sizeof(dst));
  mywrite(sockfd, src, size);
  auto it = send_connection_lock.find(sockfd);
  if(it == send_connection_lock.end()) {
    throw std::runtime_error("internal error in exrpc_rawsend");
  } else {
    pthread_mutex_unlock(it->second);
  }
}

void exrpc_rawrecv(exrpc_node& n, char* dst,
                   exrpc_ptr_t src, exrpc_count_t size) {
  int sockfd = handle_exrpc_connect_pooled(n.hostname, n.rpcport);
  exrpc_header hdr;
  hdr.type = exrpc_type::exrpc_rawrecv_type;
  hdr.funcname = std::string();
  hdr.arg_count = size;
  
  my_portable_ostream result;
  my_portable_oarchive outar(result);
  outar & hdr;
  PORTABLE_OSTREAM_TO_STRING(result, serialized_hdr);
  uint32_t hdr_size = serialized_hdr.size();
  uint32_t hdr_size_nw = htonl(hdr_size);
  mywrite(sockfd, reinterpret_cast<char*>(&hdr_size_nw), sizeof(hdr_size_nw));
  mywrite(sockfd, serialized_hdr.c_str(), hdr_size);
  mywrite(sockfd, reinterpret_cast<char*>(&src), sizeof(src));
  myread(sockfd, dst, size);
  auto it = send_connection_lock.find(sockfd);
  if(it == send_connection_lock.end()) {
    throw std::runtime_error("internal error in exrpc_rawrecv");
  } else {
    pthread_mutex_unlock(it->second);
  }
}

}
