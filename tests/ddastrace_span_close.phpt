--TEST--
ddastrace_span_close() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
function foo() {
    $retval = 'foo';
    ddastrace_span_close($retval);
    ddastrace_span_close(42);
}
foo();
?>
--EXPECT--
Called: ddastrace_span_close(): foo
Called: ddastrace_span_close()
