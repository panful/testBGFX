[CU-Production/hello_bgfx: GLFW + bgfx demo (github.com)](https://github.com/CU-Production/hello_bgfx)

[<引擎架构>-Bgfx多线程渲染 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/474396981)

[跨平台图形渲染引擎bgfx分析 | 码农家园 (codenong.com)](https://www.codenong.com/cs105888060/)

```shell
$ git clone --recursive git@github.com:Yangpan08/testBGFX.git
$ cd 3rdparty/bgfx
$ ..\bx\tools\bin\windows\genie --with-tools vs2022 // 生成bgfx的sln工程（不生成示例），需要vs2022
$ msbuild .build\projects\vs2022\bgfx.sln  /p:Configuration=Debug /p:Platform="x64" // 生成bgfx库文件，需要设置msbuild.exe的环境变量
```

