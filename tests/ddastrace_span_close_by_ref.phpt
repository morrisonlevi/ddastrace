--TEST--
ddastrace_span_close_by_ref() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
$test = ['foo'];
function &foo(array &$test) {
    $test[] = 'bar';
    return $test;
};
$result = foo($test);
var_dump($test === $result);
?>
--EXPECTF--
Called: ddastrace_span_open()
Opened span: #%d
	parent: #0
	start: %d (duration: 0)

Called: ddastrace_span_close_by_ref()
Closed span (by ref): #%d
	parent: #0
	start: %d (duration: %d)

bool(true)
