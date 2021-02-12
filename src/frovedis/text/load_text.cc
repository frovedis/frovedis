#include "load_text.hpp"
#include <algorithm>

using namespace std;

namespace frovedis {

struct load_text_helper {
  load_text_helper(){}
  load_text_helper(const string& path,
                   const string& delim,
                   ssize_t start) :
    path(path), delim(delim), start(start) {}
  void operator()(vector<int>& ret, vector<size_t>& sep, vector<size_t>& len,
                  ssize_t& rend) {
    MPI_File fh;
    time_spent t(DEBUG);
    int r = MPI_File_open(frovedis_comm_rpc, const_cast<char*>(path.c_str()),
                          MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    if(r != 0) throw runtime_error("error in MPI_File_open");
    MPI_Offset file_size_;
    r = MPI_File_get_size(fh, &file_size_);
    size_t file_size = static_cast<size_t>(file_size_);
    if(r != 0) throw runtime_error("error in MPI_File_get_size");
    size_t nodes = get_nodesize();
    int self = get_selfid();
    bool read_until_middle;
    ssize_t end;
    if(rend == -1 || rend >= file_size) {
      read_until_middle = false; end = file_size;
    } else {
      read_until_middle = true; end = rend; 
    }
    if(start < 0 || start > file_size)
      throw runtime_error("load_text_helper: start position is invalid");
    size_t read_file_size = end - start;
    size_t each_size = ceil_div(read_file_size, nodes);
    size_t delim_len = delim.size();
    if(each_size < delim_len)
      throw runtime_error("delimiter size is larger than each part");
    size_t myoff, myend;
    if(self == 0) myoff = start;
    else myoff = min(each_size * self - (delim_len-1) + start, size_t(end));
    if(self == nodes - 1) myend = end;
    else myend = min(each_size * (self + 1) + start, size_t(end));
    size_t mysize = myend - myoff;
    char datarep[] = "native";
    MPI_File_set_view(fh, myoff, MPI_CHAR, MPI_CHAR, datarep,
                      MPI_INFO_NULL);

    MPI_Status st;
    string buf;
    buf.resize(mysize);
    char* bufp = &buf[0];
    size_t maxsize = 0;
    if(sizeof(size_t) == 8)
      MPI_Allreduce(&mysize, &maxsize, 1, MPI_UNSIGNED_LONG_LONG, MPI_MAX,
                    frovedis_comm_rpc);
    else
      MPI_Allreduce(&mysize, &maxsize, 1, MPI_UNSIGNED_LONG, MPI_MAX,
                    frovedis_comm_rpc);
    size_t numiter = ceil_div(maxsize, mpi_max_count);
    size_t to_read = mysize;
    size_t pos = 0;
    for(size_t i = 0; i < numiter; i++) {
      size_t read_size = min(to_read, mpi_max_count);
      r = MPI_File_read_all(fh, bufp + pos, read_size, MPI_CHAR, &st);
      if(r != 0) throw runtime_error("error in MPI_File_read");
      pos += read_size;
      if(to_read > mpi_max_count) to_read -= mpi_max_count;
      else to_read = 0;
    }
    MPI_File_close(&fh);
    t.show("load_text: MPI_File_read_all time: ");
    auto intstring = char_to_int(buf);
    {string tmp; tmp.swap(buf);} // deallocate buf
    auto delimstart = find(intstring, delim);
    int to_receive;
    int send_size;
    if(self == 0) {
      send_size = 0;
      to_receive = 1;
    } else {
      if(delimstart.size() > 0) {
        // start: delim-1, end: delimstart[0] + delim
        send_size = delimstart[0] + 1;
        to_receive = 1;
      } else {
        to_receive = 0;
        send_size = mysize > (delim_len-1) ? mysize - (delim_len-1) : 0;
      }
    }
    vector<int> send_sizev(nodes);
    MPI_Allgather(&send_size, 1, MPI_INT, send_sizev.data(), 1, MPI_INT,
                  frovedis_comm_rpc);
    vector<int> to_receivev(nodes);
    MPI_Allgather(&to_receive, 1, MPI_INT, to_receivev.data(), 1, MPI_INT,
                  frovedis_comm_rpc);
    vector<int> sendbuf(send_size);
    auto sendbufp = sendbuf.data();
    auto from = intstring.data() + delim_len - 1;
    for(size_t i = 0; i < send_size; i++) {
      sendbufp[i] = from[i];
    }
    vector<size_t> sendcounts(nodes);
    if(self > 0) {
      for(int i = self-1; i >= 0; i--) {
        if(to_receivev[i]) {
          sendcounts[i] = send_size;
          break;
        }
      }
    }
    vector<size_t> sendcountsv(nodes*nodes);
    if(sizeof(size_t) == 8) {
      MPI_Allgather(sendcounts.data(), nodes, MPI_UNSIGNED_LONG_LONG,
                    sendcountsv.data(), nodes, MPI_UNSIGNED_LONG_LONG,
                    frovedis_comm_rpc);
    } else {
      MPI_Allgather(sendcounts.data(), nodes, MPI_UNSIGNED_LONG,
                    sendcountsv.data(), nodes, MPI_UNSIGNED_LONG,
                    frovedis_comm_rpc);
    }
    vector<size_t> recvcounts(nodes);
    auto recvcountsp = recvcounts.data();
    auto sendcountsvp = sendcountsv.data();
    for(size_t i = 0; i < nodes; i++) {
      recvcountsp[i] = sendcountsvp[nodes * i + self];
    }
    size_t total_recv = 0;
    for(size_t i = 0; i < nodes; i++) {
      total_recv += recvcounts[i];
    }
    vector<int> recvbuf(total_recv);
    std::vector<size_t> sdispls(nodes), rdispls(nodes);
    for(int i = 1; i < nodes; i++) {
      sdispls[i] = sdispls[i-1] + sendcounts[i-1];
      rdispls[i] = rdispls[i-1] + recvcounts[i-1];
    }
    auto recvbufp = recvbuf.data();
    large_alltoallv(sizeof(int),
                    reinterpret_cast<char*>(sendbufp), sendcounts, sdispls,
                    reinterpret_cast<char*>(recvbufp), recvcounts, rdispls,
                    frovedis_comm_rpc);
    if(self == 0) {
      ret.resize(mysize + total_recv);
      auto retp = ret.data();
      auto from = intstring.data();
      for(size_t i = 0; i < mysize; i++) retp[i] = from[i];
      retp += mysize;
      for(size_t i = 0; i < total_recv; i++) retp[i] = recvbufp[i];
      auto delimstart_size = delimstart.size();
      sep.resize(delimstart_size + 1); // first is always 0
      auto sepp = sep.data(); 
      auto delimstartp = delimstart.data();
      for(size_t i = 0; i < delimstart_size; i++) {
        sepp[i+1] = delimstartp[i] + delim_len;
      }
    } else {
      if(mysize > delim_len-1) {
        ret.resize(mysize - (delim_len-1) - send_size + total_recv);
        auto retp = ret.data();
        auto from = intstring.data() + delim_len - 1 + send_size;
        for(size_t i = 0; i < mysize - (delim_len-1) - send_size; i++)
          retp[i] = from[i];
        retp += mysize - (delim_len-1) - send_size;
        for(size_t i = 0; i < total_recv; i++) retp[i] = recvbufp[i];
        auto delimstart_size = delimstart.size();
        if(delimstart_size > 0) {
          sep.resize(delimstart_size);
          auto sepp = sep.data();
          auto delimstartp = delimstart.data();
          auto off = delimstartp[0];
          for(size_t i = 0; i < delimstart_size; i++) {
            sepp[i] = delimstartp[i] - off;
          }
        } else {
          sep.resize(0);
        }
      } else {
        ret.resize(0);
        sep.resize(0);
      }
    }
    auto sep_size = sep.size();
    len.resize(sep_size);
    if(sep_size > 0) {
      auto lenp = len.data();
      auto sepp = sep.data();
      for(size_t i = 0; i < sep_size - 1; i++) {
        lenp[i] = sepp[i+1] - sepp[i] - delim_len;
      }
      int lastdelim = true;
      auto ret_size = ret.size();
      // if ret_size < delim_len (if ret_size == 0), lastdelim != true
      if(ret_size >= delim_len) {
        for(size_t i = 0; i < delim_len; i++) {
          if(ret[ret.size() - delim_len + i] != static_cast<int>(delim[i])) {
            lastdelim = false;
            break;
          }
        }
      } else lastdelim = false;
      if(lastdelim) {
        lenp[sep_size - 1] = ret.size() - sepp[sep_size - 1] - delim_len;
      } else {
        lenp[sep_size - 1] = ret.size() - sepp[sep_size - 1];
      }
      int last_receive = get_nodesize() - 1; // should be signed
      for(; last_receive >= 0; last_receive--) {
        if(to_receivev[last_receive]) break;
      }
      if(get_selfid() == last_receive) {
        if(lastdelim) {
          sep.pop_back();
          len.pop_back();
        } else if(ret_size == 0) {
          sep.resize(0);
          len.resize(0);
        } else if(read_until_middle) {
          auto to_truncate = ret.size() - sepp[sep_size - 1];
          rend -= to_truncate;
          ret.resize(ret.size() - to_truncate);
          len.pop_back();
          sep.pop_back();
        }
      }
    }
    t.show("load_text: find delimiter and align time: ");
  }
  string path;
  string delim;
  ssize_t start;
  SERIALIZE(path, delim, start)
};

// sep: start of the item
node_local<vector<int>>
load_text(const string& path, const string& delim,
          node_local<vector<size_t>>& sep,
          node_local<vector<size_t>>& len) {
  auto ret = make_node_local_allocate<vector<int>>();
  auto bend = broadcast(ssize_t(-1)); // dummy
  ret.mapv(load_text_helper(path, delim, 0), sep, len, bend);
  return ret;
}

node_local<vector<int>>
load_text_separate(const string& path, const string& delim,
                   node_local<vector<size_t>>& sep,
                   node_local<vector<size_t>>& len,
                   ssize_t start, ssize_t& end){
  auto ret = make_node_local_allocate<vector<int>>();
  auto bend = broadcast(end);
  ret.mapv(load_text_helper(path, delim, start), sep, len, bend);
  auto newend = bend.gather();
  for(size_t i = 0; i < newend.size(); i++) {
    if(newend[i] != end) {end = newend[i]; break;}
  }
  return ret;
}

vector<int>
load_text_local(const string& path, const string& delim,
                vector<size_t>& sep, vector<size_t>& len){
  auto vchar = loadbinary_local<char>(path);
  vector<int> intstring(vchar.size());
  char_to_int(vchar.data(), vchar.size(), intstring.data());
  auto delimstart = find(intstring, delim);
  auto delimstart_size = delimstart.size();
  sep.resize(delimstart_size + 1); // first is always 0
  auto sepp = sep.data(); 
  auto delimstartp = delimstart.data();
  auto delim_len = delim.size();
  for(size_t i = 0; i < delimstart_size; i++) {
    sepp[i+1] = delimstartp[i] + delim_len;
  }
  auto sep_size = sep.size();
  len.resize(sep_size);
  auto lenp = len.data();
  for(size_t i = 0; i < sep_size - 1; i++) {
    lenp[i] = sepp[i+1] - sepp[i] - delim_len;
  }
  int lastdelim = true;
  auto ret_size = intstring.size();
  // if ret_size < delim_len, lastdelim != true
  if(ret_size >= delim_len) {
    for(size_t i = 0; i < delim_len; i++) {
      if(intstring[intstring.size() - delim_len + i]
         != static_cast<int>(delim[i])) {
        lastdelim = false;
        break;
      }
    }
  } else lastdelim = false;
  if(lastdelim) {
    lenp[sep_size - 1] = intstring.size() - sepp[sep_size - 1] - delim_len;
    sep.pop_back();
    len.pop_back();
  } else {
    lenp[sep_size - 1] = intstring.size() - sepp[sep_size - 1];
  }
  return intstring;
}

}

