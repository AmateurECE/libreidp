# Plugin Loading

> TODO: Fuzzy version matching?

# Nginx HTTP Auth

Where LibreIdP has been configured with the NGINX\_HTTP\_AUTH FRONTEND, when an
HTTP REQUEST is received, if a valid authorization header is not provided,
LibreIdP shall indicate an HTTP 401 Unauthorized RESPONSE with a
WWW-Authenticate header that points to a user authentication form.
