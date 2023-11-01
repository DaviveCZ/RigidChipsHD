#pragma warning(disable : 4996)
#define STRICT
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <D3DX8.h>
#include <DXErr8.h>
#include <tchar.h>
#include <dinput.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DFile.h"
#include "D3DUtil.h"
#include "DIUtil.h"
#include "DMUtil.h"
#include "DSUtil.h"
#include "DXUtil.h"
#include "D3Dtypes.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <locale.h>
#include "string.h"
#include "ctype.h"
#include "readData.hpp"
#include "luaScript.hpp"
#include "luaSub.hpp"

//メモリリーク検出用
#include <crtdbg.h>  
#ifdef _DEBUG 
#ifdef __MEMLEAKCHECK 
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__) 
#define malloc(p1) _malloc_dbg((p1),_NORMAL_BLOCK,__FILE__,__LINE__) 
#define __MEMLEAKCHECK
#endif
#endif 
//--メモリリーク検出用

extern char szUpdateFileName[];
extern char szUpdateFileName0[];
extern GFloat luaL3dx,luaL3dy,luaL3dz;
extern int luaGraColor;
extern int randTime;
extern DWORD LoadlibDummy;

// Luaに登録するCの関数は、intを返し、lua_State*を受け取るものである必要がある。
// 返す値は戻り値の数。この場合数値を1個返す（スタックに積む）ので1。

int luaPrint(lua_State *L)
{
	int n=lua_gettop(L);
	if(n<1) return 0;
	int a=(int)lua_tonumber(L, 1);
	if(a<0 || a>=GOUTPUTMAX) return 0;

	__luaPrintSub(L, 2, n, ScriptOutput[a], GOUTPUTMAXCHAR);
	return 0;
}
lua_State *luaScriptInit(char *buff) {
	ScriptErrorStr[0]='\0';
	ScriptErrorCode=0;
	setlocale(LC_ALL, "");
    //
	lua_State *L = lua_open();  /* create state */
	// 関数を登録する(C13)
    lua_register(L, "_CCD32", luaGetCCD32);
    lua_register(L, "_GETFOGCOLOR", luaGetFogColor);
	// 関数を登録する(v1.5C)
	lua_register(L, "_RANGECHIP",luaGetRangeChip);
    lua_register(L, "_GETSPEEDLIM", luaGetSpeedLimit);
    lua_register(L, "_GETSPPEDLIM", luaGetSpeedLimit); //誤字ってたけどそのまま残す
    lua_register(L, "_GETFOGRANGE", luaGetFogRange);
    lua_register(L, "_GETNAMESIZE", luaGetNameSize);
    lua_register(L, "_GETMAKERSIZE", luaGetMakerSize);
    lua_register(L, "_FACEDATA", luaGetFaceData);
	lua_register(L, "_RANGE",luaGetRange);
    lua_register(L, "_PLAYEREXTTIME", luaGetPlayerExtTime);
    lua_register(L, "_PLAYERPOS",luaGetPlayerPos);
    lua_register(L, "_OVX",luaGetObjVx);
    lua_register(L, "_OVY",luaGetObjVy);
    lua_register(L, "_OVZ",luaGetObjVz);
	//ｽﾋﾟ互換用
    lua_register(L, "InitMTRand", luaRandInit);
    lua_register(L, "MTRand", luaRand);
    lua_register(L, "GetTickCount", luaGettimeGetTime);
    lua_register(L, "GetHostName", luaGetHostName);
    lua_register(L, "GetPortNumber", luaGetPortNumber);
    lua_register(L, "GetFogDepth", luaGetFogRange);
	lua_register(L, "_CHAT", luaGetLastChat);
    lua_register(L, "_GETVIEW", luaGetView);
    lua_register(L, "_GETVIEWUP", luaGetViewUp);
    lua_register(L, "_GETVIEWTYPE", luaGetViewType);
    lua_register(L, "_SETVIEWTYPE", luaSetViewType);
    lua_register(L, "_GETVIEWZOOM", luaGetViewZoom);
    lua_register(L, "_SETVIEWZOOM", luaSetViewZoom);
	//ﾀﾞﾐｰ関数
	if(LoadlibDummy ) lua_register(L, "loadlib",luaDummyFuncFunc);
    lua_register(L, "_SETLIMITVELOCITY", luaDummyFunc1);
    lua_register(L, "SetFogDepth", luaDummyFunc1);
	lua_register(L, "_NTICKS",luaDummyFunc1);
	lua_register(L, "_SETVIEW",luaDummyFunc1);
	lua_register(L, "_SETVIEWUP",luaDummyFunc1);
	lua_register(L, "_EXFOPEN",luaDummyFunc1);
	lua_register(L, "_EXFCLOSE",luaDummyFunc1);
	lua_register(L, "_EXFPUTS",luaDummyFunc1);
	lua_register(L, "_EXFGETS",luaDummyFunc1);
    lua_register(L, "_GETY", luaDummyFunc1);
    lua_register(L, "_STICKS", luaGetTickCount);
    lua_register(L, "_SKEY", luaDummyFunc1);
    lua_register(L, "_SKEYUP", luaDummyFunc1);
    lua_register(L, "_SKEYDOWN", luaDummyFunc1);
    lua_register(L, "_SENDALL", luaDummyFunc1);
    lua_register(L, "_RECEIVE", luaDummyFuncStr);
    lua_register(L, "_RECEIVECLEAR", luaDummyFunc0);
	
	
    // 関数を登録する
    lua_register(L, "out", luaPrint);

	lua_register(L, "_MX", luaGetMouseX);
	lua_register(L, "_MY", luaGetMouseY);
	lua_register(L, "_ML", luaGetMouseL);
	lua_register(L, "_MR", luaGetMouseR);
	lua_register(L, "_MM", luaGetMouseM);

	lua_register(L, "_DT", luaGetDt);
    lua_register(L, "_FPS", luaGetFps);
    lua_register(L, "_BASE", luaGetBase);
    lua_register(L, "_TICKS", luaGetTickCount);
    lua_register(L, "_SETTICKS", luaSetTicks);
    lua_register(L, "_KEY", luaKey);
    lua_register(L, "_KEYDOWN", luaKeyDown);
    lua_register(L, "_KEYUP", luaKeyUp);
    lua_register(L, "_ANALOG", luaAnalog);
    lua_register(L, "_HAT", luaHat);
    lua_register(L, "_CHIPS", luaChips);
    lua_register(L, "_WEIGHT", luaWeight);
    lua_register(L, "_WIDTH", luaGetWidth);
    lua_register(L, "_HEIGHT", luaGetHeight);
    lua_register(L, "_FACE", luaGetFaces);
    lua_register(L, "_ZOOM", luaSetCCDZoom);
    lua_register(L, "_OX", luaObjPosx);
    lua_register(L, "_OY", luaObjPosy);
    lua_register(L, "_OZ", luaObjPosz);
    lua_register(L, "_X", luaPosx);
    lua_register(L, "_Y", luaPosy);
    lua_register(L, "_Z", luaPosz);
    lua_register(L, "_H", luaGetY);
    lua_register(L, "_AX", luaAx);
    lua_register(L, "_AY", luaAy);
    lua_register(L, "_AZ", luaAz);
    lua_register(L, "_EX", luaEx);
    lua_register(L, "_EY", luaEy);
    lua_register(L, "_EZ", luaEz);
    lua_register(L, "_GX", luaGx);
    lua_register(L, "_GY", luaGy);
    lua_register(L, "_GZ", luaGz);
    lua_register(L, "_XX", luaXx);
    lua_register(L, "_XY", luaXy);
    lua_register(L, "_XZ", luaXz);
    lua_register(L, "_YX", luaYx);
    lua_register(L, "_YY", luaYy);
    lua_register(L, "_YZ", luaYz);
    lua_register(L, "_ZX", luaZx);
    lua_register(L, "_ZY", luaZy);
    lua_register(L, "_ZZ", luaZz);
    lua_register(L, "_QX", luaQx);
    lua_register(L, "_QY", luaQy);
    lua_register(L, "_QZ", luaQz);
    lua_register(L, "_QW", luaQw);
    lua_register(L, "_RX", luaRx);
    lua_register(L, "_RY", luaRy);
    lua_register(L, "_RZ", luaRz);
    lua_register(L, "_LX", luaLx);
    lua_register(L, "_LY", luaLy);
    lua_register(L, "_LZ", luaLz);
    lua_register(L, "_VX", luaVx);
    lua_register(L, "_VY", luaVy);
    lua_register(L, "_VZ", luaVz);
    lua_register(L, "_FX", luaFx);
    lua_register(L, "_FY", luaFy);
    lua_register(L, "_FZ", luaFz);
    lua_register(L, "_WX", luaWx);
    lua_register(L, "_WY", luaWy);
    lua_register(L, "_WZ", luaWz);
    lua_register(L, "_CCD", luaGetCCD);
    lua_register(L, "_RED", luaGetCCDRed);
    lua_register(L, "_GREEN", luaGetCCDGreen);
    lua_register(L, "_BLUE", luaGetCCDBlue);
    lua_register(L, "_BYE", luaUnLinkBye);
    lua_register(L, "_SPLIT", luaUnLink);
    lua_register(L, "_RND", luaRnd);
    lua_register(L, "_TODEG", luaToDeg);
    lua_register(L, "_TORAD", luaToRad);
    lua_register(L, "_TYPE", luaGetType);
    lua_register(L, "_OPTION", luaGetOption);
    lua_register(L, "_EFFECT", luaGetEffect);
    lua_register(L, "_USER1", luaGetUserEffect);
    lua_register(L, "_USER2", luaGetUserOption);
    lua_register(L, "_DIR", luaGetDir);
    lua_register(L, "_ANGLE", luaGetAngle);
    lua_register(L, "_POWER", luaGetPower);
    lua_register(L, "_SPRING", luaGetSpring);
    lua_register(L, "_DAMPER", luaGetDamper);
    lua_register(L, "_BRAKE", luaGetBrake);
    lua_register(L, "_COLOR", luaGetColor);
    lua_register(L, "_PARENT", luaGetParent);
    lua_register(L, "_TOP", luaGetTop);
    lua_register(L, "_M", luaGetM);
    lua_register(L, "_I", luaGetI);
    lua_register(L, "_MOBJ", luaGetObjM);
    lua_register(L, "_IOBJ", luaGetObjI);
    lua_register(L, "_E", luaGetEnergy);
	lua_register(L, "_T",luaGetTolerant);
	lua_register(L, "_MOVE3D",luaMove3D);
	lua_register(L, "_LINE3D",luaLine3D);
	lua_register(L, "_MOVE2D",luaMove2D);
	lua_register(L, "_LINE2D",luaLine2D);
	lua_register(L, "_SETCOLOR",luaSetColor);
	lua_register(L, "_PLAYERS",luaGetPlayers);
	lua_register(L, "_PLAYERHOSTID",luaGetPlayerHostID);
	lua_register(L, "_PLAYERMYID",luaGetPlayerMyID);
	lua_register(L, "_PLAYERID",luaGetPlayerID);
	lua_register(L, "_PLAYERCHIPS",luaGetPlayerChips);
	lua_register(L, "_PLAYERCRUSHES",luaGetPlayerCrushes);
	lua_register(L, "_PLAYERRESETS",luaGetPlayerResets);
	lua_register(L, "_PLAYERINITS",luaGetPlayerInits);
	lua_register(L, "_PLAYERCOLOR",luaGetPlayerColor);
	lua_register(L, "_PLAYERX",luaGetPlayerX);
	lua_register(L, "_PLAYERY",luaGetPlayerY);
	lua_register(L, "_PLAYERZ",luaGetPlayerZ);
	lua_register(L, "_PLAYERARMS",luaGetPlayerArms);
	lua_register(L, "_PLAYERYFORCES",luaGetPlayerYForces);
	lua_register(L, "_PLAYERNAME",luaGetPlayerName);
	lua_register(L, "_FUEL",luaGetFuel);
	lua_register(L, "_FUELMAX",luaGetFuelMax);

	//---LUA_PATHをLua側へ登録
	const char* lua_path=getenv("LUA_PATH_5_0"); //この辺の定数がｿｰｽに残ってないので仕方なくﾏｼﾞｯｸﾅﾝﾊﾞｰで検索
	if(!lua_path) lua_path = getenv("LUA_PATH");
	if(lua_path){
		lua_pushstring(L, lua_path); lua_setglobal(L, "LUA_PATH");

	}
	//--------
	// ﾓﾃﾞﾙﾌｧｲﾙﾊﾟｽをLua側へ登録
	{
		char* itr=szUpdateFileName;
		while(strcmp(itr, szUpdateFileName0) && *itr++){}

		lua_pushlstring(L, szUpdateFileName, itr-szUpdateFileName); lua_setglobal(L, "MODEL_PATH");
		lua_pushlstring(L, szUpdateFileName0, strlen(szUpdateFileName0)); lua_setglobal(L, "MODEL_NAME");
	}
	//--------
	// EXEﾊﾟｽをLua側へ登録
	{
		//----------------------------
		//ファイルパス登録 (ホストアプリケーション,DLL)  FATとかWin32APIの仕様としては、_MAX_PATH"文字"制限なので_MAX_PATH"byte"のバッファじゃまずいんでは…?   ということで全部*2しとこ
		char tmpBuff[_MAX_PATH*2];   //パス組立用一時バッファ

		char szDrive[_MAX_DRIVE*2];	// ドライブ名格納領域 
		char szPath[_MAX_DIR*2];		// パス名格納領域 
		char szTitle[_MAX_FNAME*2];	// ファイルタイトル格納領域 
		char szExt[_MAX_EXT*2];		// ファイル拡張子格納領域 

		//------------
		GetModuleFileNameA(NULL, tmpBuff, sizeof(tmpBuff)); //ここで返るパスはUNCパスだったり8.3だったり  モジュール呼び出し時に使った文字列依存らしい
		_splitpath_s(tmpBuff, szDrive, szPath, szTitle, szExt);

		lua_pushfstring(L, "%s%s", szDrive, szPath); lua_setglobal(L, "EXE_PATH");
		lua_pushfstring(L, "%s%s", szTitle, szExt); lua_setglobal(L, "EXE_NAME");
		//------------
	}

	//--------
	luaL3dx=luaL3dy=luaL3dz=0.0f;
	luaGraColor=0xffffff;
	//グローバル変数の登録
	for(int i=0;i<VarCount;i++) {
		lua_pushnumber(L,ValList[i].Val);
		lua_setglobal(L,ValList[i].Name);
	}
	//スクリプトをセットする
      luaopen_string(L);
      luaopen_base(L);
      luaopen_table(L);
      luaopen_math(L);
	  luaopen_io(L);


	//----------------------------
	{ //LUA_INIT.lua実行
		//L={}
		lua_getglobal(L, "EXE_PATH");
		lua_pushstring(L, "LUA_INIT.lua");
		lua_concat(L, 2);
		//L={filepath}
		int err;
		if((err=luaL_loadfile(L, lua_tostring(L, -1)))) {
			if(err==LUA_ERRFILE){
				lua_pop(L,1);
				err=luaL_loadstring(L,
					"io=nil;"
					"os=nil;"
				);
			}
		} else {
			if (!LoadlibDummy) luaopen_loadlib(L); //loadlibはﾀﾞﾐｰでなくLUA_INIT有効な経路でのみ有効に
		}
		lua_remove(L,-2);
		//L={LUA_INIT}

		lua_pushcfunction(L, luaErrMsgHandler);
		lua_insert(L, -2);
		//L={msgh,LUA_INIT}
		if(err || (err=lua_pcall(L, 0, 0, -2))){
			ScriptErrorCode = -1;
			snprintf(ScriptErrorStr, GOUTPUTMAXCHAR, "%s\n", lua_tostring(L, -1));
			lua_close(L);
			return NULL;
		}
		lua_pop(L,1);
	}//L={}
	//----------------------------

	//この辺りでｼﾅﾘｵLua呼び出し位置置けば全関数潰せるかな?
	//int e=lua_dobuffer (L,buff,strlen(buff),szUpdateFileName0); 
			//  lua_dobufferはundocumentedな関数(Lua4.0時代の残骸?)のようで、挙動が他のdo***系と異なりｽﾀｯｸにｴﾗｰｵﾌﾞｼﾞｪｸﾄを残さずstderrに吐き出す(または_G["_ALEAT"] 変数が関数の場合はそれを呼び出す)模様
	lua_pushcfunction(L, luaErrMsgHandler);
	if(luaL_loadbuffer(L,buff,strlen(buff),szUpdateFileName0) || lua_pcall(L, 0, 0, -2)) {
		ScriptErrorCode=-1;
		snprintf(ScriptErrorStr, GOUTPUTMAXCHAR,"%s\n",lua_tostring(L,-1));
		lua_close(L);
		return NULL;
	}
	lua_pop(L,1);
	//L={}
	return L;
}
void luaScriptEnd(lua_State *L) {
	if(L==NULL) return;
	lua_close(L);  /* create state */
	return;
}

int luaScriptRun (lua_State *L,char *funcName) {
//	int status;
//  struct Smain s;
	//グローバル変数の登録
	for(int i=0;i<VarCount;i++) {
		lua_pushnumber(L,ValList[i].Val);
		lua_setglobal(L,ValList[i].Name);
	}
	// グローバルテーブルからmain関数を拾ってスタックに積む
	lua_pushstring(L, funcName);
	lua_gettable (L, LUA_GLOBALSINDEX);
	if(lua_topointer(L,-1)==NULL) {//もし関数がないなら
		lua_pop ( L, 1 );//スタックを戻す
		if(strcmp(funcName,"OnFrame")==0) {
			lua_pushstring(L, "main");
			lua_gettable (L, LUA_GLOBALSINDEX);
		}
		else return 0;
	}
    // 関数を呼ぶ。lua_callの第2引数は渡す引数の数、第3引数は戻り値の数。
    // 関数とその引数はスタックから取り除かれ、戻り値がスタックに残る。
	lua_pushcfunction(L, luaErrMsgHandler);
	lua_insert(L,-2);
    ScriptErrorCode=lua_pcall(L,0,0,-2);
	if(ScriptErrorCode){
		snprintf(ScriptErrorStr, GOUTPUTMAXCHAR,"%s\n",lua_tostring(L,-1));
		lua_pop(L,1);//使い終わったｴﾗｰﾒｯｾｰｼﾞを捨てる
	}
	lua_pop(L, 1); //ﾒｯｾｰｼﾞﾊﾝﾄﾞﾗを捨てる
	for(int i=0;i<VarCount;i++) {
		lua_pushstring( L , ValList[i].Name ); // (1) Luaの変数名toCを指定
		lua_gettable( L , LUA_GLOBALSINDEX ); // (2)と(3)の動作
		double v = lua_tonumber( L , -1 ); // (4) 仮想スタックのトップ内容(toCの中身)を数値型で取り出す
		if(ValList[i].Val!=v) ValList[i].Updated=true;
		ValList[i].Val=(GFloat)v;
		lua_pop( L , 1 ); // (5) 取り出したら仮想スタックを1個popする
		if(ValList[i].Val>ValList[i].Max) ValList[i].Val=ValList[i].Max;
		if(ValList[i].Val<ValList[i].Min) ValList[i].Val=ValList[i].Min;
	}
 	luaUpdateVal();
	randTime+=rand()%3+1;
	if(randTime>20000000) randTime=0;
	return 0;
}

