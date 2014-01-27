//!@todo: files_t: Make file rotation.
//! Sample config:
/*!
Rotate by size:
"rotate": {
    "pattern": "%(filename)s.log.%N",
    "backups" 5,
    "size": 1000000
}

Rotate by datetime:
"rotate": {
    "backups" 5,
    "pattern": "%(filename)s.log.%Y%M%d",
    "every": "d" [m, H, a, d, w, M, y]
}

Rotate both:
"rotate": {
    "backups" 5,
    "pattern": "%(filename)s.log.%N.%Y%M%d",
    "size": 1000000,
    "every": "d" [m, H, a, d, w, M, y]
}
*/

//!@todo: example: Make stdout/string example with demonstration of formatting other attribute.
//!@todo: example: File rotating.

//!@todo: benchmark: File logging comparing with boost::log.
//!@todo: benchmark: Socket logging with json.

//!@todo: files_t: Make file naming by pattern.

//!@todo: performance: Experiment with std::ostringstream or format library for performance check.
//!@todo: performance: Current naive implementation of timestamp formatter is suck and have large performance troubles. Fix it.

//!@todo: api: Make fallback logger. Make it configurable.
//!@todo: aux: Make internal exception class with attribute keeping, e.g. line, file or path.
//!@todo: api: Maybe squish repository_t::init and ::configure methods?
//!@todo: msgpack_t: Attribute mappings.
//!@todo: socket_t: Make asynchronous TCP backend.
