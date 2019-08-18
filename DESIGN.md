# High Level Design

The engine has a `zend_ast_process` hook which is triggered before code compilation. This project uses this hook to perform AST transformations on function and method declarations to insert calls to open and close spans. This proof of concept attempts to instrument all userland functions and methods but not closures, though its AST recursion is potentially incomplete.

The transformation are:

  - Insert a call to `ddastrace_span_open()` at the openning of the function.
  - Wrap the original content in `try { ... } catch (Throwable $ex) { throw ddastrace_span_close_exception($ex);`.
  - Insert a call at the close of the function to `ddastrace_span_close_void()`.
  - Wrap the return statement expressions in `ddastrace_span_close(...)` or `ddastrace_span_close_by_ref(...)` depending on whether the traced function returns by reference or not.

## Example Transformation

Here is a `to_iterator` function, which takes an `iterable` and returns an `Iterator`.

```php
function to_iterator(iterable $input): Iterator {
    if (is_array($input))
        return new ArrayIterator($input);
    else if ($input instanceof Iterator)
        return $input;
    else
        return new IteratorIterator($input);
}
```

This extension will convert it into this, minus whitespace differences:

```php
function to_iterator(iterable $input): Iterator {
    ddastrace_span_open();
    try {
        if (is_array($input))
            return ddastrace_span_close(new ArrayIterator($input));
        else if ($input instanceof Iterator)
            return ddastrace_span_close($input);
        else
            return ddastrace_span_close(new IteratorIterator($input));
    } catch (Throwable $ex) {
        throw ddastrace_span_close_exception($ex);
    }
    ddastrace_span_close_void();
}
```

## Why not try/finally? Wouldn't that be easier?

Yes, it would be much easier! However, it would not have access to the return value, which is something we are interested in logging in some cases.
