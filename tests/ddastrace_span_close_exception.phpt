--TEST--
ddastrace_span_close_exception() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
function foo() {
    throw new Exception('Oops!', 13);
}

try {
    foo();
    echo "No exception thrown!\n";
} catch (Exception $ex) {
    if ($ex->getCode() === 13 && $ex->getMessage() === 'Oops!') {
        echo "All is well.\n";
    } else {
        echo "Caught an exception, but not the right one.\n";
    }
}
?>
--EXPECTF--
Called: ddastrace_span_open()
Opened span: #%d
	parent: #0
	start: %d (duration: 0)

Called: ddastrace_span_close_exception()
Closed span (exception): #%d
	parent: #0
	start: %d (duration: %d)

All is well.

