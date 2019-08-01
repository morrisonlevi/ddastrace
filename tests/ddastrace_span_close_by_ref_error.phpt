--TEST--
ddastrace_span_close_by_ref() Error cases
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
try {
    $retval = 'foo';
    ddastrace_span_close_by_ref($retval);
} catch (Error $e) {
    echo $e->getMessage() . "\n";
}
?>
--EXPECT--
ddastrace_span_close_by_ref() called without function context
