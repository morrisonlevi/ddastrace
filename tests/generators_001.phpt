--TEST--
generators should still be fine
--SKIPIF--
<?php if (!extension_loaded("ddastrace")) die("skip: ddastrace required"); ?>
--FILE--
<?php

function gen() {
	yield 1;
	yield 2;
}

foreach (gen() as $value) {
	echo "$value\n";
}

?>
--EXPECTF--
Called: ddastrace_span_open()
Opened span: #%d
	parent: #0
	start: %d (duration: 0)

1
2
Called: ddastrace_span_close_void()
Closed span (void): #%d
	parent: #0
	start: %d (duration: %d)
