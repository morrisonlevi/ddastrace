--TEST--
ddastrace_span_close_exception() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
function foo() {
    $exception = new Exception('Oops!');
    $result = ddastrace_span_close_exception($exception);
    var_dump($result->getMessage());
}
foo();
?>
--EXPECT--
Called: ddastrace_span_close_exception()
string(5) "Oops!"
