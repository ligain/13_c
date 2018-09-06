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

.....
----------------------------------------------------------------------
Ran 5 tests in 0.001s

OK

Write to: test.pb.gz
Start parsing dict: {'device': {'type': 'idfa', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7c'}, 'lat': 67.7835424444, 'lon': -22.8044005471, 'apps': [1, 2, 3, 4]}
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'lat': 42, 'lon': -42, 'apps': [1, 2]}
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'lat': 42, 'lon': -42, 'apps': []}
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'apps': [1]}
Read from: test.pb.gz
Read from: wrong_file.pb.gz

Write to: test.pb.gz
Start parsing dict: {'device': {'type': 'idfa', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7c'}, 'lat': 67.7835424444, 'lon': -22.8044005471, 'apps': [1, 2, 3, 4]}
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'lat': 42, 'lon': -42, 'apps': [1, 2]}
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'lat': 42, 'lon': -42, 'apps': []}
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'apps': [1]}
bytes_written: 268

Write to: test.pb.gz
Start parsing dict: {'device': 42, 'apps': [1]}
key device is not a dict
An error has occurred parsing an item: {'device': 42, 'apps': [1]} Skipping...

Write to: test.pb.gz
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'apps': ['wrong_type']}
An error has occurred parsing an item: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'apps': ['wrong_type']} Skipping...

Write to: test.pb.gz
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'apps': [1, 2, 'wrong_type']}
An error has occurred parsing an item: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'apps': [1, 2, 'wrong_type']} Skipping...

Write to: test.pb.gz
Start parsing dict: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'apps': [1.0]}
An error has occurred parsing an item: {'device': {'type': 'gaid', 'id': 'e7e1a50c0ec2747ca56cd9e1558c0d7d'}, 'apps': [1.0]} Skipping...
 ---> 2352bb0adc10
Removing intermediate container 9a7c0586ab18
Successfully built 2352bb0adc10

```


### Project Goals
The code is written for educational purposes.