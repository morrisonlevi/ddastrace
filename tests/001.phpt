--TEST--
Check if ddastrace is loaded
--SKIPIF--
<?php
if (!extension_loaded('ddastrace')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "ddastrace" is available';
?>
--EXPECT--
The extension "ddastrace" is available
