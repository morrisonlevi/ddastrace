--TEST--
ddastrace_span_close_by_ref() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
function &foo($test) {
    debug_zval_dump($test);
    return $test;
}
$test = 'foo';
$result = foo($test);
debug_zval_dump($result);
?>
--EXPECTF--
Called: ddastrace_span_open()
Opened span: #%d
	parent: #0
	start: %d (duration: 0)

string(3) "foo" refcount(1)
Called: ddastrace_span_close_by_ref()
Closed span (void): #%d
	parent: #0
	start: %d (duration: %d)

string(3) "foo" refcount(1)
