# Model Terms

> TODO: NGINX\_HTTP\_AUTH FRONTEND

> TODO: AUTHORIZATION HEADER VALIDITY CHECK

> TODO: PLUGIN LOAD DIRECTORIES

> TODO: REQUESTED PLUGINS

> TODO: Plugin "instantiation" in configuration files, to prevent coupling
> plugins to other plugins--and resolving plugins by instance name. E.g.:
```
plugins:
    nginx_http_auth:
        name: auth
        header_check: test
    jwt:
        name: test
```
> In this case, when nginx_http_auth loads and requests a header_check plugin,
> it can search for only the interfaces that match the key "test" (in this
> case, only one). This allows the user to configure a specific interface for
> each plugin.

# Plugin Loading

When LibreIdP transitions to the INITIALIZATION state, LibreIdP shall
iteratively search the configured PLUGIN LOAD DIRECTORIES for REQUESTED
PLUGINS.

> TODO: Fuzzy version matching?

> TODO: Open plugin file and check version compatibility?

# Nginx HTTP Auth

Where LibreIdP has been configured with the NGINX\_HTTP\_AUTH FRONTEND, when an
HTTP request is received, if the AUTHORIZATION HEADER VALIDITY CHECK fails for
the request, LibreIdP shall indicate an HTTP 401 Unauthorized HTTP response
with a WWW-Authenticate header that points to a user authentication form.
