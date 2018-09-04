# Protobuf Serializer
 Simple python C extension which serializes an iterable object with dictionaries into protobuf serialized strings `*.gz` file.
### Run
You should have installed Docker on your system
```
$ docker --version
Docker version 1.13.1, build 092cba3
$ git clone https://github.com/ligain/13_c
$ cd 13_c/
$ docker build -t c_13 .
...
running build_ext
building 'pb' extension
gcc -pthread -fno-strict-aliasing -O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector-strong --param=ssp-buffer-size=4 -grecord-gcc-switches -m64 -mtune=generic -D_GNU_SOURCE -fPIC -fwrapv -DNDEBUG -O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector-strong --param=ssp-buffer-size=4 -grecord-gcc-switches -m64 -mtune=generic -D_GNU_SOURCE -fPIC -fwrapv -fPIC -I/usr/include/google/protobuf-c/ -I/usr/include/python2.7 -c pb.c -o build/temp.linux-x86_64-2.7/pb.o -g -std=gnu99
gcc -pthread -fno-strict-aliasing -O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector-strong --param=ssp-buffer-size=4 -grecord-gcc-switches -m64 -mtune=generic -D_GNU_SOURCE -fPIC -fwrapv -DNDEBUG -O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector-strong --param=ssp-buffer-size=4 -grecord-gcc-switches -m64 -mtune=generic -D_GNU_SOURCE -fPIC -fwrapv -fPIC -I/usr/include/google/protobuf-c/ -I/usr/include/python2.7 -c deviceapps.pb-c.c -o build/temp.linux-x86_64-2.7/deviceapps.pb-c.o -g -std=gnu99
gcc -pthread -shared -Wl,-z,relro build/temp.linux-x86_64-2.7/pb.o build/temp.linux-x86_64-2.7/deviceapps.pb-c.o -L/usr/lib -L/usr/lib64 -lprotobuf-c -lz -lpython2.7 -o build/lib.linux-x86_64-2.7/pb.so
copying build/lib.linux-x86_64-2.7/pb.so ->
running test
running egg_info
writing pb.egg-info/PKG-INFO
writing top-level names to pb.egg-info/top_level.txt
writing dependency_links to pb.egg-info/dependency_links.txt
reading manifest file 'pb.egg-info/SOURCES.txt'
writing manifest file 'pb.egg-info/SOURCES.txt'
running build_ext
copying build/lib.linux-x86_64-2.7/pb.so ->
..
----------------------------------------------------------------------
Ran 2 tests in 0.000s

OK

Write to: test.pb.gz
Start parsing dict: {'device': {'type': 'idfa', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7c'}, 'lat': 67.7835424444, 'lon': -22.8044005471, 'apps': [1, 2, 3, 4]}
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'lat': 42, 'lon': -42, 'apps': [1, 2]}
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'lat': 42, 'lon': -42, 'apps': []}
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'apps': [1]}
Read from: test.pb.gz
dict from file: {'device': {'type': 'idfa', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7c'}, 'lat': 67.7835424444, 'apps': [1, 2, 3, 4], 'lon': -22.8044005471}
dict from file: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'lat': 42.0, 'apps': [1, 2], 'lon': -42.0}
dict from file: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'lat': 42.0, 'apps': [], 'lon': -42.0}
dict from file: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'apps': [1]}

Write to: test.pb.gz
Start parsing dict: {'device': {'type': 'idfa', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7c'}, 'lat': 67.7835424444, 'lon': -22.8044005471, 'apps': [1, 2, 3, 4]}
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'lat': 42, 'lon': -42, 'apps': [1, 2]}
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'lat': 42, 'lon': -42, 'apps': []}
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'apps': [1]}
bytes_written: 268
 ---> 8ce8da4b5046
Removing intermediate container 2aaf5ed8a330
Successfully built 8ce8da4b5046
```


### Project Goals
The code is written for educational purposes.