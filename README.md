Nginx module to compare the value of user-defined variables and request.
==========

[![License](http://img.shields.io/badge/license-BSD-brightgreen.svg)](https://github.com/vozlt/nginx-module-udc/blob/master/LICENSE)

Nginx module to compare the value of user-defined variables and request.

## Dependencies
* nginx

## Installation

1. Clone the git repository.

  ```
  shell> git clone git://github.com/vozlt/nginx-module-udc.git
  ```

2. Add the module to the build configuration by adding 
  `--add-module=/path/to/nginx-module-udc`

3. Build the nginx binary.

4. Install the nginx binary.

## Synopsis

```Nginx
location /user-define-check {
    user_define_check               on;
    user_define_check_out_type      json;
    user_define_check_allow_text    "true";
    user_define_check_deny_text     "false";
    user_define_check_agent         "USER_DEFINED_0";
    user_define_check_agent         "USER_DEFINED_1";
    user_define_check_agent         "USER_DEFINED_2";
}
```

## Description
This is an Nginx module that compare the value of user-defined variables and value of request.
This module returns the compared result to string that is specific type.
The types as follows:
* JSON
* TEXT

## Directives

### user_define_check

| -                  | -                          |
| ------------------ | -------------------------- |
| **Syntax**         | user_define_check [on\|off] |
| **Default**        | off                        |
| **Context**        | http, server, location     |

Description: Enables or disables user_define_check function.

### user_define_check_out_type

| -                  | -                                      |
| ------------------ | -------------------------------------- |
| **Syntax**         | user_define_check_out_type [json\|text] |
| **Default**        | json                                   |
| **Context**        | http, server, location, limit_except   |

Description: The output type of result.

### user_define_check_allow_text

| -                  | -                                    |
| ------------------ | ------------------------------------ |
| **Syntax**         | user_define_check_allow_text string  |
| **Default**        | true                                 |
| **Context**        | http, server, location, limit_except |

Description: The string of success result.

### user_define_check_deny_text

| -                  | -                                    |
| ------------------ | ------------------------------------ |
| **Syntax**         | user_define_check_deny_text string   |
| **Default**        | false                                |
| **Context**        | http, server, location, limit_except |

Description: The string of failure result.

### user_define_check_agent

| -                  | -                                    |
| ------------------ | ------------------------------------ |
| **Syntax**         | user_define_check_agent string       |
| **Default**        | -                                    |
| **Context**        | http, server, location, limit_except |

Description: The string to check.

## Testing

```
shell> (echo -en "GET /user-define-check?USER_DEFINED_0 HTTP/1.1\r\nHost:localhost\r\n\r\n"; sleep 1) | nc localhost 80
```
```
{"status":"true"}
```

```
shell> (echo -en "GET /user-define-check?USER_DEFINED_3 HTTP/1.1\r\nHost:localhost\r\n\r\n"; sleep 1) | nc localhost 80
```
```
{"status":"false"}
```

## Author
YoungJoo.Kim [<vozlt@vozlt.com>]
