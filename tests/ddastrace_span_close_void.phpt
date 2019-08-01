--TEST--
ddastrace_span_close_void() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
function foo() {
    ddastrace_span_open();
    usleep(50);
    ddastrace_span_close_void();
}
foo();
?>
--EXPECTF--
Called: ddastrace_span_open()
Opened span: #%d
	parent: #0
	start: %d
	duration: 0

Called: ddastrace_span_close_void()
Closed span: #%d
	parent: #0
	start: %d
	duration: %d
