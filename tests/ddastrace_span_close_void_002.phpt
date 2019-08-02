--TEST--
ddastrace_span_open() & ddastrace_span_close_void() Basic functionality
--SKIPIF--
<?php if (!extension_loaded('ddastrace')) echo 'skip: ddastrace required'; ?>
--FILE--
<?php
function print_hi_if(bool $if): void {
	if ($if) {
		echo "hi\n";
		return;
	}
	echo "Uh-oh! Early return was broken.\n";
}
try {
	print_hi_if(true);
} catch (Throwable $ex) {
	echo "Oops :( ", get_class($ex), ": " , $ex->getMessage(), "\n";
}
?>
--EXPECTF--
Called: ddastrace_span_open()
Opened span: #%d
	parent: #0
	start: %d (duration: 0)

hi
Called: ddastrace_span_close_void()
Closed span (void): #%d
	parent: #0
	start: %d (duration: %d)
