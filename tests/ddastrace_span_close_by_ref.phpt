--TEST--
ddastrace_span_close_by_ref() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
$test = ['foo'];
$foo = function &() use (&$test) {
    $test[] = 'bar';
    debug_zval_dump($test);
    return $test;
};
debug_zval_dump($test);
$result = $foo($test);
debug_zval_dump($result);
var_dump($test === $result);
?>
--EXPECTF--
array(1) refcount(3){
  [0]=>
  string(3) "foo" refcount(1)
}
Called: ddastrace_span_open()
Opened span: #%d
	parent: #0
	start: %d (duration: 0)

array(2) refcount(2){
  [0]=>
  string(3) "foo" refcount(2)
  [1]=>
  string(3) "bar" refcount(1)
}
Called: ddastrace_span_close_by_ref()
Closed span (by ref): #%d
	parent: #0
	start: %d (duration: %d)

array(2) refcount(3){
  [0]=>
  string(3) "foo" refcount(2)
  [1]=>
  string(3) "bar" refcount(1)
}
bool(true)
