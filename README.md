# Logging-Subsystem

Logging subsystem

You should develop a custom logging subsystem. The list of key requirements:

1. Logging should support writing to multiple files in a multithreading environment. For example, one thread can write in the first.log file, and another thread can write in the second.log file at the same time.

2. Writing in log files should be done in a background thread for minimization of performance impact on a thread that initiated a log writing.

3. The logging subsystem every day should reopen a log file. If nobody tries to write any information to log the file, reopening is not needed.

4. For pointing a log file, you should use a relative path, for example, "/core/execution/orders" in this case "<root>\core\execution\yyyy-MM-dd_orders.log" file will be created. Where <root> is an absolute path to the root directory of all logs.

5. If any directory or file doesn't exist, the logging subsystem has to create them.

6. If any logging file exists, the logging subsystem should continue writing to it.

7. The logging library should provide a simple and convenient way for formatting complex messages.

8. All errors inside the logging subsystem should be ignored silently.

9. Format of the log record:

yyyy-MM-dd HH:mm:ss.fff, <thread id>: <message>, where:

yyyy-MM-dd HH:mm:ss.fff - UTC date and time of a message, when it was formatted by the code

<thread ID> - an ID of a thread that initiated writing to a log file

<message> - is any message

10. You should provide a way of changing a path to the root of all logs on the fly (without restarting the application).
