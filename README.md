# AST-based tracer for APM

This proof of concept project uses the PHP `zend_ast_process` hook to use the AST to find function definitions and insert function calls that will open and close spans, as well as exception information.

See the [DESIGN.md](DESIGN.md) file for the design and more detailed information.

## Credits

This proof of concept was written by Levi Morrison and Sammy Powers during Datadog's Hackathon in August 2019. Thanks to Joe Watkins for providing some feedback.
