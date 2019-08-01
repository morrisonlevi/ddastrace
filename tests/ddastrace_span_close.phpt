--TEST--
ddastrace_span_close() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
$retval = 'foo';
ddastrace_span_close($retval);
ddastrace_span_close(42);
?>
--EXPECT--
Called: ddastrace_span_close(): foo
Called: ddastrace_span_close()
