--TEST--
ddastrace_span_close() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
function foo() {
    echo "Foo!\n";
    return "retfoo\n";
}
echo foo();
?>
--EXPECTF--
Called: ddastrace_span_open()
Opened span: #%d
	parent: #0
	start: %d (duration: 0)

Foo!
retfoo
Called: ddastrace_span_close(): retfoo
Closed span (void): #%d
	parent: #0
	start: %d (duration: %d)
