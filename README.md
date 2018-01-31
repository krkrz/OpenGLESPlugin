# OpenGL ES 2.0/3.0 をプラグインから使用するサンプル
吉里吉里Zマルチプラットフォーム版でプラグインからOpenGL ES 2.0/3.0 APIを使用するためのサンプルです。

## 使用上の注意
TVPeglGetProcAddressは、Windowsでは、Windowクラスのコンストラクタが呼ばれた後でないと機能しないことに注意。  
プラグインの読み込みと同時にTVPInitializeOpenGLPluginで初期化する場合は、Windowクラスのコンストラクタ呼び出し以降にリンクすること。  
プラグインに初期化メソッドを持つ場合は、初期化メソッドの呼び出しをWindowクラスのコンストラクタ呼び出し以降にする。  
プラグインクラスのインスタンス化時にTVPInitializeOpenGLPluginを毎回呼び出す実装にして、Windowクラスのコンストラクタ内でクラス生成するような実装であれば問題ない。  

eglGetProcAddressが必要であれば、以下の定義のみあればよい。  
これはOpgnGLESInit.cpp内で定義されているので、リンクしておけばlib内で同名メソッド経由でAPIを取得している場合は問題なく呼び出せる。

```cpp
extern "C" void* eglGetProcAddress( const char* name ) {
    return TVPeglGetProcAddress(name);
}
```

## 状態の復帰
吉里吉里Z本体側はCanvasクラスがOpenGL ES2.0/3.0の状態を持ち、変化があった場合に適用する形になっているため、プラグイン内で状態を変更して描画を行った場合は、呼び出しから戻る前に以前の状態に戻す必要がある。  
もし、状態を元に戻していない場合、以降の描画が期待したように行われなくなってしまう。  
