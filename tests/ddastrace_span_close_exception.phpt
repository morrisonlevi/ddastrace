--TEST--
ddastrace_span_close_exception() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
$exception = new Exception('Oops!');
$result = ddastrace_span_close_exception($exception);
var_dump($result->getMessage());
?>
--EXPECT--
Called: ddastrace_span_close_exception()
string(5) "Oops!"
