--TEST--
ddastrace_span_open() Basic functionality
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required';
?>
--FILE--
<?php
ddastrace_span_open();
?>
--EXPECT--
Called: ddastrace_span_open()
