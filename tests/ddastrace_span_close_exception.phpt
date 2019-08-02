--TEST--
ddastrace_span_close_exception() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
function foo() {
    new Exception('Oops!');
}
foo();
?>
--EXPECTF--
Called: ddastrace_span_open()
Opened span: #%d
	parent: #0
	start: %d (duration: 0)

Called: ddastrace_span_close_exception()
Closed span (void): #%d
	parent: #0
	start: %d (duration: %d)

Uncaught exception...
