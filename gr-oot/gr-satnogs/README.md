# gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
![gr-satnogs](docs/assets/gr-satnogs.png)

gr-satnogs is an out-of-tree GNU Radio module that provides all the necessary tools
for decoding signals from various scientific and academic satellites.
It also provides blocks for debugging and experimenting with known satellite
telecommunication schemes. Pre-built packages for gr-satnogs are available for some distributions at
[open SUSE build service: librespace:satnogs](https://build.opensuse.org/project/show/home:librespace:satnogs).

## Installation

### Requirements
* GNU Radio ( >= 3.8.0 )
* CMake ( >= 3.8)
* G++ (> 4.8)
* Boost
* VOLK
* libogg
* libvorbis
* libpng
* libpng++
* nlohmann-json (>= 3.0)
* git
* swig

#### Debian / Ubuntu
To install GNU Radio 3.8 on Ubuntu based distributions that do not have already an
available package:

```bash
 sudo add-apt-repository ppa:gnuradio/gnuradio-releases
 sudo apt-get update
```

For Debian you can use the the [stable](https://build.opensuse.org/project/show/home:librespace:satnogs)
repository available from [Open Build Service](https://openbuildservice.org/).
For latest experimental software you can use the 
[unstable](https://build.opensuse.org/project/show/home:librespace:satnogs-unstable)
repository.

Then to build from source use:

```bash 
sudo apt install -y \
      libboost-dev \
      libitpp-dev \
      libboost-date-time-dev \
      libboost-filesystem-dev \
      libboost-program-options-dev \
      libboost-system-dev \
      libboost-thread-dev \
      libboost-regex-dev \
      libboost-test-dev \
      swig \
      cmake \
      build-essential \
      pkg-config \
      gnuradio-dev \
      libconfig++-dev \
      libgmp-dev \
      liborc-0.4-0 \
      liborc-0.4-dev \
      liborc-0.4-dev-bin \
      nlohmann-json3-dev \
      libpng++-dev \
      libvorbis-dev \
      git
```
#### openSUSE
```bash
sudo zypper in -y \
      boost-devel \
      itpp-devel \
      libboost_filesystem-devel \
      libboost_system-devel \
      libboost_thread-devel \
      libboost_program_options-devel \
      libboost_regex-devel \
      libboost_test-devel \
      python3 \
      python3-devel \
      swig \
      cmake \
      gcc-c++ \
      gcc \
      gnuradio \
      gnuradio-devel \
      gmp-devel \
      libmpir-devel \
      liborc-0_4-0 \
      orc \
      log4cpp-devel \
      nlohmann_json-devel \
      git
```

### Installation from source

```bash
git clone https://gitlab.com/librespacefoundation/satnogs/gr-satnogs.git
cd gr-satnogs
mkdir build
cd build
cmake ..
make -j $(nproc --all)
sudo make install
```

If this is the first time you are building the gr-satnogs module run
```bash
sudo ldconfig
```

#### Advanced
By default, the **SatNOGS** module will use the default installation prefix.
This highly depends on the Linux distribution. You can use the `CMAKE_INSTALL_PREFIX`
variable to alter the default installation path.
E.g:

```bash
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
```

Also, by default the build system enables a set of blocks used for debugging
during the development. The enable/disable switch is controlled through the
`INCLUDE_DEBUG_BLOCKS` boolean variable. If for example, you want to disable the
debugging blocks, the **CMake** command would be:

```bash
cmake -DINCLUDE_DEBUG_BLOCKS=OFF ..
```

Another common control option is the library suffix of the Linux distribution.
There are distributions like Fedora, openSUSE, e.t.c that the their 64-bit version
use the `lib64` folder to store the 64-bit versions of their dynamic libraries.
On the other hand, distributions like Ubuntu do the exact opposite. They use
`lib` directory for the libraries of the native architecture and place the 32-bit versions
on the `lib32` directory. In any case the correct library directory suffix
can be specified with the `LIB_SUFFIX` variable. For example (and in case you're using 
openSuSE Tumbleweed):

```bash
cmake -DLIB_SUFFIX=64 -DCMAKE_INSTALL_PREFIX=/usr -DINCLUDE_DEBUG_BLOCKS=OFF ..
```

will install the libraries at the `/usr/lib64` directory.

## Development Guide
The development is performed on the `master` branch.
For special cases where a team of developers should work an a common feature,
maintainers may add a special branch on the repository.
However, it will be removed at the time it will be merged on the `master` branch.
All developers should derive the `master` branch for their feature branches and merge
requests should also issued at this branch.
Developers should ensure that do **not** alter the CMake version tags in any way.
It is a responsibility of the maintainers team.  

Before submitting a new merge request, rebase the `master` branch and
confirm that the automated CI tests have successfully completed for all platforms
mandated by the `.gitlab-ci.yml` recipe.
Make sure also that you sign your work following the rules described in the
[CONTRIBUTING.md](CONTRIBUTING.md).

### Coding style
For the C++ code, `gr-satnogs` uses a slightly modified version of the 
**Stroustrup** style, which is a nicer adaptation of the well known K&R style.
In addition, we decided to decrease the indentation from 4 to 2 spaces.
This choice was made mainly to avoid breaking statements with long namespaces.
We also found ourselves, that with smaller indentation we use more descriptive
variable names, avoiding frustrating abbreviations without phoenixes etc. 

At the root directory of the project there is the `astyle` options 
file `.astylerc` containing the proper configuration.
Developers can import this configuration to their favorite editor. 
In addition the `hooks/pre-commit` file contains a Git hook, 
that can be used to perform before every commit, code style formatting
with `astyle` and the `.astylerc` parameters.
To enable this hook developers should copy the hook at their `.git/hooks` 
directory. 
Failing to comply with the coding style described by the `.astylerc` 
will result to failure of the automated tests running on our CI services. 
So make sure that you either import on your editor the coding style rules 
or use the `pre-commit` Git hook.


Regarding the naming of files and variables, we use the underscore naming 
convention (`do_this`) instead of camel cases (`DoNotDoThis`). 
Exception to this rule is the CMake module filenames. In addition,
all private variables of a C++ class, should start with the prefix 
`d_` allowing the developers to spot easily private members of the object.

### Adding a new Satellite Decoder
With the new architecture, adding a new satellite has become an easy and straight
forward task.
The decoders are implemented using the following approach.

There is a generic block called `frame_decoder`.
This block should not be altered at any case. If you find yourself in a situation
that you need to apply modifications on this block, raise an issue on the 
[issue tracker](https://gitlab.com/librespacefoundation/satnogs/gr-satnogs/issues)
The `frame_decoder` block accepts two parameters. A `satnogs::decoder`
object and the item size of the input stream. Internally, the `frame_decoder`
invokes the `decode()` method of the `satnogs::decoder` class.

The `satnogs::decoder` class, is a virtual class providing a generic API that
every derived decoder class should implement. 
The core of this class is the

```cpp
decoder_status_t 
decode(const void *in, int len)
```
method. This method accepts an input buffer `in`. The type of the items depends
on the implementation. It also takes the `len` argument specifying the number
of items available in the `in` buffer.
The method returns a `decoder_status_t` class object.

```cpp
class decoder_status
{
public:
  int                   consumed;       
  bool                  decode_success; 
  pmt::pmt_t            data;

  decoder_status () :
    consumed(0),
    decode_success(false),
    data(pmt::make_dict())
  {
  }
};

typedef class decoder_status decoder_status_t;
```
The class contains three fields that allow the `frame_decoder` block to operate
continuously, without any further assistance. It is responsibility of the derived
decoder class to properly set the values to these fields. 

* The `consumed` class should contain the number of items consumed during the
`decode()` method invocation. It is ok to consume 0, less than `len` or `len`
items but not more.
* `decode_success` should be set to true only if a frame was successfully
decoded and its data are available on the `data` field.
* `data` field is a `pmt::pmt_t` dictionary containing the decoded data and other 
information regarding it, using the `gr-satnogs` metadata format. More about them
in the [Metadata](#metadata) section

### Adding a new Satellite Flowgraph
In most cases, a satellite decoder needs also a GNU Radio flowgraph
with the necessary blocks that will process the input signal before passing it
to the decoder (e.g. filtering, clock recovery, etc).
We maintain these flowgraphs inside a separate repository called [satnogs-flowgraphs](https://gitlab.com/librespacefoundation/satnogs/satnogs-flowgraphs).
Please follow the development guide of this repository to submit your flowgraphs.

### Metadata
Each decoder generates a `pmt::pmt_t` dictionary containing the decoded data and
other information regarding the decoded frame.
The `gr::satnogs::metadata` class provides a set of commonly used metadata
keys.
The table below describes some of them:

| Key | Description |
| --- | ----------- |
| pdu         | This string field contains the decoded data in base64 form |
| time        | The time at which the frame was received. Time is represented in an ISO 8601 string with microsecond accuracy |
| crc_valid   | Boolean indicating if the CRC check has been successfully passed |
| freq_offset | Float value indicating the frequency offset observed |
| corrected_bits | `uint64_t` with the number of corrected bits |
| symbol_erasures | `uint64_t` with the number of erased symbols |
| sample_start | `uint64_t` with the sample index at which the decoder identified the start of the frame |
| sample_cnt | `uint64_t` with the number of samples of a valid frame.  `sample_start + sample_cnt` specify the sample index at the end of the frame |
| snr        | `float` with the estimated SNR in dB

The method `Json::Value
metadata::to_json(const pmt::pmt_t& m)` is converts the dictionary `m`
into a valid JSON object. There is also the `std::string
metadata::keys()` static method which returns a list with the available
metadata keys. This method is also available in Python through the Swig interface.
 For example:

```python
$ python
>>> import satnogs
>>> satnogs.metadata.keys()
'[pdu, crc_valid, freq_offset, corrected_bits, time, sample_start, sample_cnt, symbol_erasures]'
>>> 
```

Using the `json_converter` block, developers can convert a `pmt::pmt_t` 
dictionary of a decoder into a `pmt::pmt_t` blob, 
containing the raw bytes of the JSON string, which then can be passed to a UDP
sink targeting the `satnogs-client`.
The `json_converter` block accepts also a string that may be used to inject 
an arbitrary number of additional information under the `extra` JSON field.
Of course, this string should be in a JSON valid format.

For example, such a JSON string with information on the extra field could be like

```json
{
    "corrected_bits" : 0,
    "extra" : 
    {
        "x" : 3,
        "y" : "test"
    },
    "pdu" : "igAg7nRAOCAniUMAtIoAAAAAAAAAAAAAAABNJ4kfAFD4wwAfAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==",
    "symbol_erasures" : 0,
    "time" : "2019-09-11T15:39:13.514138Z"
}
```


### Release Policy
The `gr-satnogs` OOT module uses the GNU Radio style `major.api.minor.patch`
versioning scheme.

* Patches, bug fixes that do not add any new features nor affect in anyway
`satnogs-flowgraphs` advance the `patch` version.
* Improvements and new features that do not break backwards compatibility with
`satnogs-flowgraphs` advance the `minor` version.
* The `api` indicates changes that require modifications
on `satnogs-flowgraps` and break backwards compatibility.
* `major` version advances when the are huge changes on the entire codebase which.
Such changes break backwards compatibility with `satnogs-flowgraphs`.

For every release change a tag with the corresponding version is created.
Releases can be retrieved by the 
[tags](https://gitlab.com/librespacefoundation/satnogs/gr-satnogs/tags) page.

## Website and Contact
For more information about SatNOGS please visit our [site](https://satnogs.org/)
and our [community forums](https://community.libre.space).
You can also chat with the SatNOGS community at 
[https://riot.im/app/#/room/#satnogs:matrix.org](https://riot.im/app/#/room/#satnogs:matrix.org),
or on IRC at `#satnogs` on Freenode.
For chatting around the development and for watching the changes in project's gitlab repositories,
join in [https://riot.im/app/#/room/#satnogs-dev:matrix.org](https://riot.im/app/#/room/#satnogs-dev:matrix.org)
or the IRC channel `#satnogs-dev` on Freenode.  

## License

![SatNOGS](docs/assets/SatNOGS-logo-vertical-black.png)
![Libre Space Foundation](docs/assets/LSF_HD_Horizontal_Color1-300x66.png) 
&copy; 2016-2019 [Libre Space Foundation](https://libre.space).

Licensed under the [GPLv3](LICENSE).

