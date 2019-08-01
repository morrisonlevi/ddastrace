--TEST--
ddastrace_span_close_by_ref() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
function foo() {
    $retval = 'foo';
    $retref = &$retval;
    $result = ddastrace_span_close_by_ref($retval);
    debug_zval_dump($result);
}
foo();
?>
--EXPECT--
Called: ddastrace_span_close_by_ref()
string(3) "foo" refcount(1)
