--TEST--
ddastrace_span_open() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
function foo() {
    ddastrace_span_open();
}
foo();
?>
--EXPECT--
Called: ddastrace_span_open()
