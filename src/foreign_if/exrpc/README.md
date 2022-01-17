% EXRPC

# 1. Introduction

This interface is for communication between the frovedis server and
its client. Users should not use this interface directly, since it
would change in the future. This documentation is for developers. 

# 2. Basic interface

At frovedis server side, a binary (./a.out or mpirun -np ... ./a.out)
will be executed that waits for TCP/IP connection.

In the main function, you need to call “expose” to expose a function
to RPC, like:

    expose(function_to_be_called);
    expose(another_function_to_be_called);
    ...

The argument of the functions should be *reference*. For example, 
not

    int foo(int a, int b){return a + b;}

but 

    int foo(int& a, int& b){return a + b;}

This is macro and registers the function as its *name* (== string). So
the client side should use exactly the same name when calling.

After that, in the main function, a function to wait for RPC request
is called, like:

    init_frovedis_server(argc, argv);

Here, RPC request is waited.

At the client side, you can invoke the server using some
initialization call, like:

    auto n = invoke_frovedis_server("mpirun -np 2 ./server");

Then, from the client, exposed function can be called like:

    auto r = exrpc_async(n, some_function, some_argument);
    ... // do something in parallel
    auto val = r.get();

Here, n is an object to specify the host/port to connect that is
returned by `invoke_frovedis_server`.
The function `exrpc_async` connects to the host and send RPC request
to the host with the information of calling function and arguments. It
returns the exrpc_result type after sending the request; at this
moment, the function does not finish the execution. Then, calling
r.get() blocks until the callee side sends the result. 
If exception occurred at the server side, it is propagated to the
client and throw exception at the time of get().

In the case of `exrpc_oneway`, it does not return value, so there is
no `get` call; it blocks until the function finishes. Propagated
exeption is thrown at `exrpc_oneway`.

In the case of `exrpc_oneway_noexcept`, it does not get the exeption
information from the server. In addition, because of this, it does not
wait for the completion of the function, which is used the parallel
RPC interface that is explained below.

# 3. Parallel RPC interface

Above interface is simple, and can be used for RPC call from
the client to MPI rank 0, but it is not sufficient to send the
data in parallel from workers (in the case of Spark, for example) to
Frovedis ranks 0 to N.

For supporting this case, there is extension to the RPC interface. 
To use the extension, first the client need to call following functions:

    exrpc_info info = prepare_parallel_exrpc(n);
    std::vector<exrpc_node> nodes = get_parallel_exrpc_nodes(n, info);
    wait_parallel_exrpc(n, info);

The first function binds sockets to ephemeral (temporal) port at all
ranks. The second function returns the nodes to connect. The third
function let the nodes wait for the RPCs.

Then, the client can call RPCs for each rank:

    for(size_t i = 0; i < nodes.size(); i++) {
      exrpc_oneway(nodes[i], some_func, args);
    }

In this case, the RPC can be called only once. However, there might be
a case that RPC need to be called multiple times. For that purpose, we
provide wait_parallel_exrpc_multi function. In this case, you can
specify the number of RPC calls for each node.

    std::vector<size_t> num_rpc = {2, 3};
    wait_parallel_exrpc_multi(n, info, num_rpc);

In this case, the number of nodes is two and rank 0 waits for RPC 2
times, rank 1 waits for RPC 3 times.

Then, the client can call RPCs for each rank like this:

    for(size_t i = 0; i < nodes.size(); i++) {
      for(size_t j = 0; j < num_rpc[i]; j++) {
        exrpc_oneway(nodes[i], some_func, args);
      }
    }

# 4. Raw RPC interface

Basically, the argument of RPC is serialized with a library like YAS.
However, users might want to skip the serialization for performance.
For this usecase, there is a special RPC:

    void exrpc_rawsend(exrpc_node& n, char* src,
                       exrpc_ptr_t dst, exrpc_count_t size);
    void exrpc_rawrecv(exrpc_node& n, char* dst,
                       exrpc_ptr_t src, exrpc_count_t size);

They can just send or receive data. Here, exrpc_ptr_t is the pointer
of the server. The `size` is number of bytes. To use this function,
both client and server have the same (little) endian.

This RPC just send or receive the data and the pointer should be
obtained by other RPCs. In the sample code,

    auto rawep = exrpc_async(n, get_each_rawpointer, r).get();

returns the pointer of the data inside of the std::vector.
In this example, parallel RPC is used:

    info = prepare_parallel_exrpc(n);
    nodes = get_parallel_exrpc_nodes(n, info);
    num_rpc = {2, 2};
    wait_parallel_exrpc_multi(n, info, num_rpc);

Then, for each node, exrpc_raw_recv is called:

    vector<vector<int>> buf(2);
    for(size_t i = 0; i < nodes.size(); i++) {
      buf[i].resize(2);
      exrpc_rawrecv(nodes[i], reinterpret_cast<char*>(buf[i].data()),
                    rawep[i], sizeof(int) * 2);
      for(auto j: buf[i]) cout << j << " ";

This should show the current data. Then, exrpc_rawsend is called

      cout << endl;
      buf[i] = {100, 200};
      exrpc_rawsend(nodes[i], reinterpret_cast<char*>(buf[i].data()),
                    rawep[i], sizeof(int) * 2);
    }

This should update the contents of the data. So

    vv = exrpc_async(n, gather_sample, r).get();
    for(auto i: vv) cout << i << endl;

shouls produce output like

    100
    200
    100
    200

The `sample` directory contains sample of exrpc program.
