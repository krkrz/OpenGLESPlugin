

#ifdef _WIN32
#include <windows.h>
#endif
#include "tp_stub.h"
#include "OpenGLHeader.h"
#include <vector>

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#define STDCALL __stdcall
#else
typedef tjs_error HRESULT;
#define DLL_EXPORT
#endif

#define TJS_NATIVE_CLASSID_NAME ClassID_OpenGLESSample
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;

// バーティクスシェーダー
static const GLchar VertexShaderScript[] =
"attribute vec2 a_pos;"
"uniform vec4 a_color;"
"varying vec4 v_color;"
"void main()"
"{"
"  gl_Position = vec4(a_pos, 0.0, 1.0);"
"  v_color = a_color;"
"}";
// フラグメントシェーダー
static const GLchar FragmentShaderScript[] =
"precision mediump float;"
"varying vec4 v_color;"
"void main() {"
"  gl_FragColor = v_color;"
"}";
// シェーダーをコンパイルする
GLuint CompileShader( GLenum type, const GLchar* source ) {
	GLuint shader = glCreateShader( type );
	glShaderSource( shader, 1, &source, nullptr );
	glCompileShader( shader );
	GLint compileResult;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &compileResult );
	if( compileResult == 0 ) {
		GLint infoLogLength;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &infoLogLength );
		if( infoLogLength > 1 ) {
			std::vector<GLchar> infoLog( infoLogLength );
			glGetShaderInfoLog( shader, static_cast<GLsizei>( infoLog.size() ), nullptr, &infoLog[0] );
			TVPAddLog( TJS_W( "GL : shader compilation failed:" ) + ttstr( &infoLog[0] ) );
		} else {
			TVPAddLog( TJS_W( "GL : shader compilation failed. <Empty log message>" ) );
		}
		glDeleteShader( shader );
		shader = 0;
	}
	return shader;
}
// リンク結果を検証する
GLuint CheckLinkStatusAndReturnProgram( GLuint program, bool outputErrorMessages ) {
	if( glGetError() != GL_NO_ERROR ) return 0;
	GLint linkStatus;
	glGetProgramiv( program, GL_LINK_STATUS, &linkStatus );
	if( linkStatus == 0 ) {
		if( outputErrorMessages ) {
			GLint infoLogLength;
			glGetProgramiv( program, GL_INFO_LOG_LENGTH, &infoLogLength );
			if( infoLogLength > 1 ) {
				std::vector<GLchar> infoLog( infoLogLength );
				glGetProgramInfoLog( program, static_cast<GLsizei>( infoLog.size() ), nullptr, &infoLog[0] );
				TVPAddLog( TJS_W( "GL : program link failed: " ) + ttstr( &infoLog[0] ) );
			} else {
				TVPAddLog( TJS_W( "GL : program link failed. <Empty log message>" ) );
			}
		}
		glDeleteProgram( program );
		return 0;
	}
	return program;
}
// シェーダーをコンパイル、リンクする
GLuint CompileProgram( const GLchar* vsSource, const GLchar* fsSource ) {
	GLuint program = glCreateProgram();
	GLuint vs = CompileShader( GL_VERTEX_SHADER, vsSource );
	GLuint fs = CompileShader( GL_FRAGMENT_SHADER, fsSource );
	if( vs == 0 || fs == 0 ) {
		glDeleteShader( fs );
		glDeleteShader( vs );
		glDeleteProgram( program );
		return 0;
	}
	glAttachShader( program, vs );
	glDeleteShader( vs );
	glAttachShader( program, fs );
	glDeleteShader( fs );
	glLinkProgram( program );
	return CheckLinkStatusAndReturnProgram( program, true );
}
static GLuint GLProgram = 0;
static GLint GLVertex = -1;
static GLint GLColor = -1;
//---------------------------------------------------------------------------
// tTJSNI_OpenGLESSample
//---------------------------------------------------------------------------
class tTJSNI_OpenGLESSample : public tTJSNativeInstance {
	typedef tTJSNativeInstance inherited;
	iTJSDispatch2 * Owner = nullptr; // owner object

	void initialize();
public:
	tTJSNI_OpenGLESSample(){}
	virtual ~tTJSNI_OpenGLESSample(){}
	tjs_error TJS_INTF_METHOD Construct( tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj );
	void TJS_INTF_METHOD Invalidate();
	void draw();
};

void tTJSNI_OpenGLESSample::initialize() {
	if( GLProgram ) return;

	GLProgram = CompileProgram( VertexShaderScript, FragmentShaderScript );

	glUseProgram( GLProgram );
	GLVertex = glGetAttribLocation( GLProgram, "a_pos" );
	GLColor = glGetUniformLocation( GLProgram, "a_color" );
}
tjs_error TJS_INTF_METHOD tTJSNI_OpenGLESSample::Construct( tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj ) {
	tjs_error hr = inherited::Construct( numparams, param, tjs_obj );
	if( TJS_FAILED( hr ) ) return hr;

	bool loadgl = TVPInitializeOpenGLPlugin();
	if( loadgl == false ) return TJS_E_FAIL;

	Owner = tjs_obj;
	initialize();
	return TJS_S_OK;
}
void TJS_INTF_METHOD tTJSNI_OpenGLESSample::Invalidate() {
	Owner = nullptr;
	inherited::Invalidate();
}
void tTJSNI_OpenGLESSample::draw() {
	// 描画後の状態の復元はここでは行っていない
	glUseProgram( GLProgram );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	glDisable( GL_CULL_FACE );
	glEnable( GL_BLEND );
	glBlendEquation( GL_FUNC_ADD );
	glBlendFunc( GL_ONE, GL_ONE );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	const GLfloat vertices[] = {
		-1.0f, -1.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
	};
	glUniform4f( GLColor, 1.0f, 0.0f, 0.0f, 1.0f );
	glVertexAttribPointer( GLVertex, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), vertices );
	glEnableVertexAttribArray( GLVertex );
	glDrawArrays( GL_TRIANGLES, 0, 3 );
}
static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_OpenGLESSample() {
	return new tTJSNI_OpenGLESSample();
}

//---------------------------------------------------------------------------
// tTJSNC_OpenGLESSample : OpenGLESSample TJS native class
//---------------------------------------------------------------------------
iTJSDispatch2 * TVPCreateNativeClass_OpenGLESSample() {
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin( TJS_W( "OpenGLESSample" ), Create_NI_OpenGLESSample );

	// register native methods/properties
	TJS_BEGIN_NATIVE_MEMBERS( Live2DCubism )
	TJS_DECL_EMPTY_FINALIZE_METHOD
	//----------------------------------------------------------------------
	// constructor/methods
	//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/_this, /*var.type*/tTJSNI_OpenGLESSample, /*TJS class name*/OpenGLESSample ) {
		return TJS_S_OK;
	}
	TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/OpenGLESSample )
	//----------------------------------------------------------------------
	TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/draw ) {
		TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/tTJSNI_OpenGLESSample );
		_this->draw();
		return TJS_S_OK;
	}
	TJS_END_NATIVE_METHOD_DECL(/*func. name*/draw )

	TJS_END_NATIVE_MEMBERS

	return classobj;
}
#undef TJS_NATIVE_CLASSID_NAME
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#ifdef _WIN32
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved) {
	return 1;
}
#endif
//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" DLL_EXPORT HRESULT STDCALL V2Link(iTVPFunctionExporter *exporter)
{
	// スタブの初期化(必ず記述する)
	TVPInitImportStub(exporter);

	tTJSVariant val;

	// TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	{
		//-----------------------------------------------------------------------
		// 1 まずクラスオブジェクトを作成
		iTJSDispatch2 * tjsclass = TVPCreateNativeClass_OpenGLESSample();

		// 2 tjsclass を tTJSVariant 型に変換
		val = tTJSVariant(tjsclass);

		// 3 すでに val が tjsclass を保持しているので、tjsclass は Release する
		tjsclass->Release();

		// 4 global の PropSet メソッドを用い、オブジェクトを登録する
		global->PropSet( TJS_MEMBERENSURE, TJS_W("OpenGLESSample"), nullptr, &val, global );
		//-----------------------------------------------------------------------
	}

	// - global を Release する
	global->Release();

	// val をクリアする。
	// これは必ず行う。そうしないと val が保持しているオブジェクトが Release されず、次に使う TVPPluginGlobalRefCount が正確にならない。
	val.Clear();


	// この時点での TVPPluginGlobalRefCount の値を
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	// として控えておく。TVPPluginGlobalRefCount はこのプラグイン内で管理されている tTJSDispatch 派生オブジェクトの参照カウンタの総計で、
	// 解放時にはこれと同じか、これよりも少なくなってないとならない。
	// そうなってなければ、どこか別のところで関数などが参照されていて、プラグインは解放できないと言うことになる。

	return TJS_S_OK;
}
//---------------------------------------------------------------------------
extern "C" DLL_EXPORT HRESULT STDCALL V2Unlink()
{
	// 吉里吉里側から、プラグインを解放しようとするときに呼ばれる関数。

	// もし何らかの条件でプラグインを解放できない場合はこの時点で E_FAIL を返すようにする。
	// ここでは、TVPPluginGlobalRefCount が GlobalRefCountAtInit よりも大きくなっていれば失敗ということにする。
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return TJS_E_FAIL;
		// E_FAIL が帰ると、Plugins.unlink メソッドは偽を返す

	/*
		ただし、クラスの場合、厳密に「オブジェクトが使用中である」ということを知るすべがありません。基本的には、Plugins.unlink によるプラグインの解放は
		危険であると考えてください (いったん Plugins.link でリンクしたら、最後までプラグインを解放せず、プログラム終了と同時に自動的に解放させるのが吉)。
	*/

	// - まず、TJS のグローバルオブジェクトを取得する
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// メニューは解放されないはずなので、明示的には解放しない

	// - global の DeleteMember メソッドを用い、オブジェクトを削除する
	if(global) {
		// TJS 自体が既に解放されていたときなどは global は NULL になり得るので global が NULL でないことをチェックする
		global->DeleteMember( 0, TJS_W("OpenGLESSample"), nullptr, global );
	}

	// - global を Release する
	if(global) global->Release();

	// スタブの使用終了(必ず記述する)
	TVPUninitImportStub();

	return TJS_S_OK;
}
//---------------------------------------------------------------------------
