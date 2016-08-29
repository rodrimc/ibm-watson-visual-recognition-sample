# IBM Watson Visual Recognition Sample Application

This is a sample application that IBM Watson Visual Recognition Service.
It receives as parameters the api key and the image filepath, access the classify 
service using the POST HTTP method and outputs the JSON response.

# Dependencies
* [libsoup](https://wiki.gnome.org/Projects/libsoup)

# Compiling
```
gcc -o post post.c `pkg-config libsoup-2.4 --cflags --libs`
```

# Running
```
./post <api_key> <path>
```

