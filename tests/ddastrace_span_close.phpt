--TEST--
ddastrace_span_close() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
function foo(): string {
    echo "Foo!\n";
    return "retfoo\n";
}
try {
    echo foo();
} catch (Throwable $ex) {
    echo "Oops :( ", get_class($ex), "\n";
}
?>
--EXPECTF--
Called: ddastrace_span_open()
Opened span: #%d
	parent: #0
	start: %d (duration: 0)

Foo!
Called: ddastrace_span_close(): retfoo

Closed span: #%d
	parent: #0
	start: %d (duration: %d)

retfoo

