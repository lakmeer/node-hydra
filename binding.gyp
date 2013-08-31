{
  "targets" : [
    {
      "target_name" : "hydra",
      "sources" : [ "src/node-hydra.cpp" ],
      "link_settings": {
        "libraries": [
          "/usr/local/lib/libsixense.dylib",
          "/usr/local/lib/libsixensed_x64.dylib"
        ]
      }
    }
  ]
}
