--TEST--
ddastrace_span_close_void() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
function foo() {
    ddastrace_span_close_void();
}
foo();
?>
--EXPECT--
Called: ddastrace_span_close_void()
