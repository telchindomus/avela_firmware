# `avela_strings`: fixed-capacity text

`FixedString<N>` constructs text in owned static-capacity storage without heap allocation or printf-style varargs.
It is used for labels, settings records, and protocol messages.

## Capacity and termination

N includes the terminating zero byte. `max_length()` is N−1 bytes, and `c_str()` remains zero-terminated. `append()`
copies only what fits; excess bytes are truncated. The capacity counts bytes, not Unicode characters, and truncation
does not validate a UTF-8 boundary.

Derive capacity from `capacity_for(longest-text)`, `number_capacity<Number>`, or one named owning limit.
`capacity_for()` includes the terminator. number_capacity provides space for the integer representation, sign, and
termination.

## Operations

`append()` adds a string view. `append_number()` formats an integral value with optional zero padding, preserving a
leading minus sign. `clear()` empties the string. `view()` exposes the current bytes without the terminator;
`c_str()` exposes a terminated pointer. Both remain views of owned storage and must not outlive it.

Text capacities in component references use these byte semantics. The
[coding standard](../../../../docs/developer/CODING_STANDARD.md#3-memory-no-heap-in-our-code) defines when fixed
text is required.
