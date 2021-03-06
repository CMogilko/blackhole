Blackhole - eating your logs with pleasure
==========================================
[ ![Codeship Status for 3Hren/blackhole](https://codeship.com/projects/8d0e44f0-64ac-0133-20de-4a7e5d8c8004/status?branch=master)](https://codeship.com/projects/113228)

This is pre-release for Blackhole 1.0.0. For clearness I've dropped all the code and started with empty project.

Some parts (almost all formatters and sinks) will be borrowed from v0.5 branch.

Other stuff will be rewritten completely using C++11/14 standard with minimal boost impact.

If you want stable, but deprecated version, please switch to v0.5 branch.

# Features

## Attributes

Attributes is the core feature of Blackhole. Technically speaking it's a key-value pairs escorting every logging record.

For example we have HTTP/1.1 server which produces access logs like:

```
[::] - esafronov [10/Oct/2000:13:55:36 -0700] 'GET /porn.png HTTP/1.0' 200 2326 - SUCCESS
```

It can be splitted into indexes or attributes:

```
message:   SUCCESS
host:      [::]
user:      esafronov
timestamp: 10/Oct/2000:13:55:36 -0700
method:    GET
uri:       /porn.png
protocol:  HTTP/1.0
status:    200
elapsed:   2326
```

Blackhole allows to specify any number of attributes you want, providing an ability to work with them before of while
you writing them into its final destination. For example, Elasticsearch.

## Shared library

Despite the header-only dark past now Blackhole is developing as a shared library. Such radical change of distributing
process was chosen because of many reasons.

Mainly, header-only libraries has one big disadvantage: any code change may (or not) result in recompiling all its dependencies, otherwise having weird runtime errors with symbol loading race.

The other reason was the personal aim to reduce compile time, because it was fucking huge!

Of course there are disadvantages, such as virtual function call cost and closed doors for inlining, but here my personal benchmark-driven development helped to avoid performance degradation.

## Planning

- [x] Shared library.
- [x] Inline namespaces.
- [x] Optional compile-time inline messages transformation (C++14).
  - [ ] Compile-time placeholder type checking.
  - [ ] Compile-time placeholder spec checking (?).
- [x] Python-like formatting (no printf-like formatting support) both inline and result messages.
- [x] Attributes.
- [x] Scoped attributes.
- [x] Wrappers.
- [x] Custom verbosity.
- [x] Custom attributes formatting.
- [ ] Optional asynchronous pipelining.
  - [x] Queue with block on overload.
  - [x] Queue with drop on overload (count dropped message).
  - [ ] The same but for handlers.
- [x] Formatters.
  - [x] String by pattern.
    - [ ] Optional placeholders.
    - [x] Configurable leftover placeholder.
  - [x] JSON with tree reconstruction.
- [x] Sinks.
  - [x] Colored terminal output.
  - [x] Files.
  - [x] Syslog.  
  - [x] Socket UDP.
  - [x] Socket TCP.
    - [x] Blocking.
    - [x] Non blocking.
- [ ] Scatter-gathered IO (?)
- [x] Logger builder.
- [ ] Macro with line and filename attributes.
- [x] Initializer from JSON (filename, string).
- [ ] Inflector.
- [ ] Filter category for sinks, handlers and loggers.

## Formatters

Formatters in Blackhole are responsible for converting every log record passing into some byte array representation. It can be either human-readable string, JSON tree or even [protobuf](https://github.com/google/protobuf) packed frame.

### String

String formatter provides an ability to configure your logging output using pattern mechanics with powerful
customization support.

Unlike previous Blackhole versions now string formatter uses python-like syntax for describing patterns with using `{}`
placeholders and format specifications inside. Moreover now you can specify timestamp specification directly inside the
general pattern or even format it as an microseconds number since epoch.

For example we have the given pattern:
```
[{severity:>7}] [{timestamp:{%Y-%m-%d %H:%M:%S.%f}s}] {scope}: {message}
```

After applying some log events we expect to receive something like this:
```
[  DEBUG] [2015-11-19 19:02:30.836222] accept: HTTP/1.1 GET - / - 200, 4238
[   INFO] [2015-11-19 19:02:32.106331] config: server has reload its config in 200 ms
[WARNING] [2015-11-19 19:03:12.176262] accept: HTTP/1.1 GET - /info - 404, 829
[  ERROR] [2015-11-19 19:03:12.002127] accept: HTTP/1.1 GET - /info - 503, 829
```

As you may notice the severity field is aligned to the right border (see that *>7* spec in pattern), the timestamp is
formatted using default representation with a microseconds extension and so on. Because Blackhole is all about
attributes you can place and format every custom attribute you want, as we just done with *scope* attribute.

The Blackhole supports several predefined attributes, with convenient specifications:

| Placeholder              | Description                                                            |
|--------------------------|------------------------------------------------------------------------|
|{severity:s}              | User provided severity string representation                           |
|{severity}, {severity:d}  | Numeric severity value                                                 |
|{timestamp:d}             | Number of microseconds since Unix epoch                                |
|{timestamp:{spec}s}       | String representation using *strftime* specification in UTC            |
|{timestamp:{spec}l}       | String representation using *strftime* specification in local timezone |
|{timestamp}, {timestamp:s}| The same as *{timestamp:{%Y-%m-%d %H:%M:%S.%f}s}*                      |
|{process:s}               | Process name                                                           |
|{process}, {process:d}    | PID                                                                    |
|{thread}, {thread::x}     | Thread hex id as an opaque value returned by *pthread_self(3)*         |
|{thread:s}                | Thread name or *unnnamed*                                              |
|{message}                 | Logging message                                                        |
|{...}                     | All user declared attributes                                           |

For more information please read the documentation and visit the following links:

 - http://cppformat.github.io/latest/syntax.html - general syntax.
 - http://en.cppreference.com/w/cpp/chrono/c/strftime - timestamp spec extension.

Note, that if you need to include a brace character in the literal text, it can be escaped by doubling: `{{` and `}}`.

There is a special attribute placeholder - `{...}` - which means to print all non-reserved attributes in a reverse order they were provided in a key-value manner separated by a comma. These kind of attributes can be configured using special syntax, similar with the timestamp attribute with an optional separator.

For example the following placeholder `{...:{{name}={value}:p}{\t:x}s}` results in tab separated key-value pairs like `id=42\tmethod=GET`.

For pedants there is a full placeholder grammar in EBNF:
```
Grammar     = Ph
            | OptPh
            | VarPh
Ph          = "{" Name "}"
OptPh       = "{" Name ":" Spec? "}"
VarPh       = "{...}"
            | "{...:" Ext? s "}"
Ext         = Pat
            | Sep
            | Pat Sep
            | Sep Pat
Name        = [a-zA-Z0-9_]
Spec        = Align? Width? Type
Align       = [>^<]
Width       = [1-9][0-9]*
Type        = [su]
Pat         = "{" PatSpec ":p}"
Sep         = "{" SepLit* ":s}" ("}" SepLit* ":s}")*
SepLit      = . ! (":s" | "}" | "}}" | "{" | "{{")
            | LeBrace
            | RiBrace
LeBrace     = "{{" -> "{"
RiBrace     = "}}" -> "}"
PatSpec     = (AtName | AtValue | PatLit)*
AtName      = "{name}"
            | AtNameSpec
AtNameSpec  = "{name:" AtSpec "}"
AtSpec      = Align? Width? AtType
AtType      = [sd]
AtValue     = "{value}"
            | AtValueSpec
AtValueSpec = "{value:" AtSpec "}"
PatLit      = . ! ("}" | "}}" | "{" | "{{")
            | LeBrace
            | RiBrace
```

Let's describe it more precisely. Given a complex leftover placeholder, let's parse it manually to see what Blackhole see.
Given: `{...:{{name}={value}:p}{\t:s}>50s}`.

Parameter          | Description                                                            
-------------------|--------------------------------------------------------------------------------------------------
...                | Reserved placeholder name indicating for Blackhole that this is a leftover placeholder.
:                  | Optional spec marker that is placed after placeholder name where you want to apply one of several extensions. There are pattern, separator, prefix, suffix and format extensions. All of them except format should be surrounded in curly braces.
{{name}={value}:p} | Pattern extension that describes how each attribute should be formatted using typical Blackhole notation. The suffix **:p**, that is required for extension identification, means pattern. Inside this pattern you can write any pattern you like using two available sub-placeholders for attribute name and value, for each of them a format spec can be applied using cppformat grammar. At last a format spec can be also applied to the entire placeholder, i.e. **:>50p** for example.
{\t:s}             | Separator extension for configuring each key-value pair separation. Nuff said.
{[:r}              | (Not implemented yet). Prefix extension that is prepended before entire result if it is not empty.
{]:u}              | (Not implemented yet). Suffix extension that is appended after entire result if it is not empty.
>50s               | Entire result format. See cppformat rules for specification.

### JSON.
JSON formatter provides an ability to format a logging record into a structured JSON tree with attribute handling features, like renaming, routing, mutating and much more.

Briefly using JSON formatter allows to build fully dynamic JSON trees for its further processing with various external tools, like logstash or rsyslog lefting it, however, in a human-readable manner.

Blackhole allows you to control of JSON tree building process using several predefined options.

Without options it will produce just a plain tree with zero level depth. For example for a log record with a severity of 3, message "fatal error, please try again" and a pair of attributes `{"key": 42, "ip": "[::]"}` the result string will look like:
```json
{
    "message": "fatal error, please try again",
    "severity": 3,
    "timestamp": 1449859055,
    "process": 12345,
    "thread": 57005,
    "key": 42,
    "ip": "[::]"
}
```
Using configuration parameters for this formatter you can:
- Rename parameters.
- Construct hierarchical tree using a standardized JSON pointer API. For more information please follow \ref https://tools.ietf.org/html/rfc6901.

Attributes renaming acts so much transparently as it appears: it just renames the given attribute name using the specified alternative.

Attributes routing specifies a location where the listed attributes will be placed at the tree construction. Also you can specify a default location for all attributes, which is "/" meaning root otherwise.

For example with routing `{"/fields": ["message", "severity"]}` and "/" as a default pointer the mentioned JSON will look like:
```json
{
    "fields": {
        "message": "fatal error, please try again",
        "severity": 3
    },
    "timestamp": 1449859055,
    "process": 12345,
    "thread": 57005,
    "key": 42,
    "ip": "[::]"
}
```

Attribute renaming occurs after routing, so mapping "message" => "#message" just replaces the old name with its new alternative.

To gain maximum speed at the tree construction no filtering occurs, so this formatter by default allows duplicated keys, which means invalid JSON tree (but most of parsers are fine with it). If you are really required to deal with unique keys, you can enable `unique` option, but it involves heap allocation and may slow down formatting.

Also formatter allows to automatically append a newline character at the end of the tree, which is strangely required by some consumers, like logstash.

Note, that JSON formatter formats the tree using compact style without excess spaces, tabs etc.

For convenient formatter construction a special builder class is implemented allowing to create and configure instances of this class using streaming API. For example:
```c++
auto formatter = blackhole::formatter::json_t::builder_t()
    .route("/fields", {"message", "severity", "timestamp"})
    .route("/other")
    .rename("message", "#message")
    .rename("timestamp", "#timestamp")
    .newline()
    .unique()
    .build();
```

This allows to avoid hundreds of constructors and to make a formatter creation to look eye-candy.

The full table of options:

Option      | Type              | Description
------------|-------------------|---------------
**/route** | Object of:<br>[string]<br>"\*" | Allows to configure nested tree mapping. Each key must satisfy [JSON Pointer](https://tools.ietf.org/html/rfc6901) specification and sets new attributes location in the tree. Values must be either an array of string, meaning list of attributes that are configured with new place or an "\*" literal, meaning all other attributes.
**/mapping** | Object of: [string] | Simple attribute names renaming from key to value.
**/newline** | bool             | If true, a newline will be appended to the end of the result message. The default is _false_.
**/unique**   | bool             | If true removes all backward consecutive duplicate elements from the attribute list range. For example if there are two attributes with name "name" and values "v1" and "v2" inserted, then after filtering there will be only the last inserted, i.e. "v2". The default is _false_.
**/mutate/timestamp** | string   | Replaces the timestamp field with new value by transforming it with the given _strftime_ pattern.
**/mutate/severity**  | [string] | Replaces the severity field with the string value at the current severity value.

For example:
```json
"formatter": {
    "type": "json",
    "newline": true,
    "unique": true,
    "mapping": {
        "message": "@message",
        "timestamp": "@timestamp"
    },
    "routing": {
        "": ["message", "timestamp"],
        "/fields": "*"
    },
    "mutate": {
        "timestamp": "%Y-%m-%dT%H:%M:%S.%fZ",
        "severity": ["D", "I", "W", "E"]
    }
}
```

## Sinks

### Null
Sometimes we need to just drop all logging events no matter what, for example to benchmarking purposes. For these cases there is null appender, which just ignores all records.

- Stream.
- Term.
- File.

### Socket
The socket sinks category contains sinks that write their output to a remote destination specified by a host and port. Currently the data can be sent over either TCP or UDP.

#### TCP
This appender emits formatted logging events using connected TCP socket.

| Option | Type  | Description|
|--------|:-----:|------------|
|host    |string | **Required**.<br/> The name or address of the system that is listening for log events. |
|port    |u16    | **Required**.<br/> The port on the host that is listening for log events. |

#### UDP
Nuff said.

#### Syslog
| Option    | Type  | Description                                               |
|-----------|:-----:|-----------------------------------------------------------|
|priorities |[i16]  | **Required**.<br/> Priority mapping from severity number. |

## Runtime Type Information

The library can be successfully compiled and used without RTTI (with *-fno-rtti* flag).

## Possible bottlenecks

- Timestamp formatting
 - [ ] Using system clock - can be replaces with OS specific clocks.
 - [ ] Using `gmtime` - manual `std::tm` generation without mutex shit.
 - [ ] Temporary buffer - affects, but not so much.

# Why another logging library?

That's the first question I ask myself when see *yet another silver-buller library*.

First of all, we required a logger with attributes support. Here `boost::log` was fine, but it didn't compile in our compilers. Sad. After that we've realized that one of our bottlenecks is located in logging part, that's why `boost::log` and `log4cxx` weren't fit in our requirements. Thirdly we are developing for stable, but old linux distributives with relatively old compilers that supports only basic part of C++11.

At last, but not least, all that libraries has one fatal disadvantage - [NIH](https://en.wikipedia.org/wiki/Not_invented_here).

So here we are.

To be honest, let's describe some popular logging libraries, its advantages and disadvantages as one of them may fit your requirements and you may want to use them instead. It's okay.

### Boost.LogV2
Developed by another crazy Russian programmer using dark template magic and Vodka (not sure what was first). It's a perfect and powerful library, seriously.

**Pros:**
- It's a fucking **boost**! Many people don't want to depend on another library, wishing to just `apt get install` instead.
- Have attributes too.
- Large community, less bugs.
- Highly configurable.
- Good documentation.

**Cons:**
- Sadly, but you are restricted with the latest boost versions.
- Hard to hack and extend unless you are fine with templates, template of templates and variadic templates of a templated templates with templates. Or you are Andrei Alexandrescu.
- Relatively poor performance. Higher than `log4cxx` have, but not enough for us.
- Requires RTTI.

### Log4cxx
Logging framework for C++ patterned after Apache log4j. Yeah, Java.

**Pros:**
- Absolutely zero barrier to entry. Really, you just copy-paste the code from tutorial and it works. Amazing!

**Cons:**
- Leaking.
- APR.
- Have no attributes.
- Really slow performance.
- Seems like it's not really supported now.

### Spdlog
Extremely ultra bloody fucking fast logging library. At least the documentation says that. Faster than speed of light!

But everyone knows that even the light is unable to leave from blackhole.

**Pros:**
- Really fast, I checked.
- Header only. Not sure it's an advantage, but for small projects it's fine.
- Easy to extend, because the code itself is plain, straightforward and magically easy to understand.
- No dependencies.
- Nice kitty in author's avatar.

**Cons:**
- Again no attributes, no custom filtering, no custom verbosity levels. You are restricted to functionality provided by this library, nothing more.

# Notable changes
First of all, the entire library was completely rewritten for performance reasons.

- No more attribute copying unless it's really required (for asynchronous logging for example). Nested attributes are now organized in flattened range.
- Dropped `boost::format` into the Hell. It's hard to find more slower library for formatting both in compilation stage and runtime. Instead, the perfect [cppformat](https://github.com/cppformat/cppformat) library with an own compile-time constexpr extensions is used.
- There are predefined attributes with fast read access, like `message`, `severity`, `timestmap` etc.
- With **cppformat** participation there is new Python-like format syntax using placeholder replacement.
- Severity mapping from its numeric representation to strings can now be configured from generic configuration source (from file for example).
- ...

# Requirements

- C++11/14/17 compiler (yep, using C++17 opens additional functionalities).
- Boost.Thread - for TLS.

# Development
## Git workflow

Each feature and fix is developed in a separate branch. Bugs which are discovered during development of a certain feature, may be fixed in the same branch as their parent issue. This is also true for small features.

### Branch structure:
- `master`: master branch - contains stable, working version of VM code.
- `develop`: development branch - all fixes and features are first merged here.
- `issue/<number>/<slug>` or `issue/<slug>`: for issues (both enhancement and bug fixes).
