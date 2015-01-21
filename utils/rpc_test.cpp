#include <stdio.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <functional>
#include <tuple>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;
using namespace std;

namespace dodo
{
    template <typename T>
    class HasCallOperator
    {
        typedef char _One;
        typedef struct{ char a[2]; }_Two;
        template<typename T>
        static _One hasFunc(decltype(&T::operator()));
        template<typename T>
        static _Two hasFunc(...);
    public:
        static const bool value = sizeof(hasFunc<T>(nullptr)) == sizeof(_One);
    };

    template<int Size>
    struct SizeType
    {
        typedef int TYPE;
    };
    template<>
    struct SizeType<0>
    {
        typedef char TYPE;
    };

    template<typename A>
    struct remove_const { typedef A type; };
    template<typename A>
    struct remove_const<const A> { typedef A type; };

    template<typename A>
    struct base_type { typedef A type; };
    template<typename A>
    struct base_type<A*> { typedef A type; };
    template<typename A>
    struct base_type<A&> { typedef A type; };

    class Utils
    {
    public:
        /*  反序列-从json中读取数据  */
        static  void    readJson(Document& doc, const Value& msg, char& ret)
        {
            ret = msg.GetInt();
        }

        static  void    readJson(Document& doc, const Value& msg, int& ret)
        {
            ret = msg.GetInt();
        }

        /*  TODO::因为Value无法拷贝，且没有空构造函数，所以实际上rpc function不支持Value作为参数;如果需要用json表示复杂参数，只能用字符串json代替   */
        static  void    readJson(Document& doc, const Value& msg, Value& ret)
        {
            ret.CopyFrom(msg, doc.GetAllocator());
        }

        static  void    readJson(Document& doc, const Value& msg, string& ret)
        {
            ret = msg.GetString();
        }

        static  void    readJson(Document& doc, const Value& msg, vector<int>& ret)
        {
            for (size_t i = 0; i < msg.Size(); ++i)
            {
                ret.push_back(msg[i].GetInt());
            }
        }

        static  void    readJson(Document& doc, const Value& msg, vector<string>& ret)
        {
            for (size_t i = 0; i < msg.Size(); ++i)
            {
                ret.push_back(msg[i].GetString());
            }
        }

        template<typename T>
        static  void    readJson(Document& doc, const Value& msg, vector<T>& ret)
        {
            for (size_t i = 0; i < msg.Size(); ++i)
            {
                T element;
                readJson(doc, msg[i], element);
                ret.push_back(element);
            }
        }

        template<typename T>
        static  void    readJson(Document& doc, const Value& msg, map<string, T>& ret)
        {
            for (Value::ConstMemberIterator itr = msg.MemberBegin(); itr != msg.MemberEnd(); ++itr)
            {
                T tv;
                readJson(doc, (*itr).value, tv);
                ret[(*itr).name.GetString()] = tv;
            }
        }

        template<typename T>
        static  void    readJson(Document& doc, const Value& msg, map<int, T>& ret)
        {
            for (Value::ConstMemberIterator itr = msg.MemberBegin(); itr != msg.MemberEnd(); ++itr)
            {
                T tv;
                readJson(doc, (*itr).value, tv);
                ret[atoi((*itr).name.GetString())] = tv;
            }
        }

        static  void    readJson(Document& doc, const Value& msg, map<string, string>& ret)
        {
            for (Value::ConstMemberIterator itr = msg.MemberBegin(); itr != msg.MemberEnd(); ++itr)
            {
                ret[(*itr).name.GetString()] = (*itr).value.GetString();
            }
        }

        static  void    readJson(Document& doc, const Value& msg, map<int, string>& ret)
        {
            for (Value::ConstMemberIterator itr = msg.MemberBegin(); itr != msg.MemberEnd(); ++itr)
            {
                ret[atoi((*itr).name.GetString())] = (*itr).value.GetString();
            }
        }

        static  void    readJson(Document& doc, const Value& msg, map<string, int>& ret)
        {
            for (Value::ConstMemberIterator itr = msg.MemberBegin(); itr != msg.MemberEnd(); ++itr)
            {
                ret[(*itr).name.GetString()] = (*itr).value.GetInt();
            }
        }

        template<typename T>
        static  T   readJsonByIndex(Document& doc, const Value& msg, int index)
        {
            T tmp;
            const Value& element = msg[Utils::itoa(index)];
            /*  TODO::readJson无法解决递归map和vector中的偏特化问题,所以有不必要的临时变量生成（无法利用右值引用),可用类模板解决   */
            readJson(doc,element, tmp);
            return tmp;
        }

    public:
        /*  序列化-把数据转换为json  */
        static  Value    writeJson(Document& doc, const int& value)
        {
            return Value(value);
        }

        static  Value   writeJson(Document& doc, const char* const& value)
        {
            return Value(value, doc.GetAllocator());
        }

        static  Value   writeJson(Document& doc, const string& value)
        {
            return Value(value.c_str(), doc.GetAllocator());
        }

        static  Value   writeJson(Document& doc, const Value& value)
        {
            Value ret;
            ret.CopyFrom(value, doc.GetAllocator());
            return ret;
        }

        static  Value   writeJson(Document& doc, const vector<int>& value)
        {
            Value arrayObject(kArrayType);
            for (size_t i = 0; i < value.size(); ++i)
            {
                arrayObject.PushBack(Value(value[i]), doc.GetAllocator());
            }
            return arrayObject;
        }

        static  Value   writeJson(Document& doc, const vector<string>& value)
        {
            Value arrayObject(kArrayType);
            for (size_t i = 0; i < value.size(); ++i)
            {
                arrayObject.PushBack(Value(value[i].c_str(), doc.GetAllocator()), doc.GetAllocator());
            }
            return arrayObject;
        }

        template<typename T>
        static  Value   writeJson(Document& doc, const vector<T>& value)
        {
            Value arrayObject(kArrayType);
            for (size_t i = 0; i < value.size(); ++i)
            {
                Value&& v = writeJson(doc, value[i]);
                arrayObject.PushBack(std::forward<Value&&>(v), doc.GetAllocator());
            }
            return arrayObject;
        }

        template<typename T, typename V>
        static  Value   writeJson(Document& doc, const map<T, V>& value)
        {
            Value mapObject(kObjectType);
            /*遍历此map*/
            for (map<T, V>::const_iterator it = value.begin(); it != value.end(); ++it)
            {
                /*把value序列化到map的jsonobject中,key就是它在map结构中的key*/
                Value&& v = writeJson(doc, it->second);
                mapObject.AddMember(GenericValue<UTF8<>>(Utils::itoa(it->first), doc.GetAllocator()), std::forward<Value&&>(v), doc.GetAllocator());
            }

            /*把此map添加到msg中*/
            return mapObject;
        }

        static  Value   writeJson(Document& doc, const map<string, string>& value)
        {
            Value mapObject(kObjectType);
            map<string, string>::const_iterator itend = value.end();
            for (map<string, string>::const_iterator it = value.begin(); it != itend; ++it)
            {
                mapObject.AddMember(GenericValue<UTF8<>>(it->first.c_str(), doc.GetAllocator()), Value(it->second.c_str(), doc.GetAllocator()), doc.GetAllocator());
            }
            return mapObject;
        }

        static  Value   writeJson(Document& doc, const map<int, string>& value)
        {
            Value mapObject(kObjectType);
            map<int, string>::const_iterator itend = value.end();
            for (map<int, string>::const_iterator it = value.begin(); it != itend; ++it)
            {
                mapObject.AddMember(GenericValue<UTF8<>>(Utils::itoa((*it).first), doc.GetAllocator()), Value(it->second.c_str(), doc.GetAllocator()), doc.GetAllocator());
            }
            return mapObject;
        }

        static  Value   writeJson(Document& doc, const map<string, int>& value)
        {
            Value mapObject(kObjectType);
            map<string, int>::const_iterator itend = value.end();
            for (map<string, int>::const_iterator it = value.begin(); it != itend; ++it)
            {
                mapObject.AddMember(GenericValue<UTF8<>>(it->first.c_str(), doc.GetAllocator()), Value(it->second), doc.GetAllocator());
            }
            return mapObject;
        }

        template<typename T>
        static  void    writeJsonByIndex(Document& doc, Value& msg, const T& t, int index)
        {
            Value&& v = writeJson(doc, t);
            msg.AddMember(GenericValue<UTF8<>>(Utils::itoa(index), doc.GetAllocator()), std::forward<Value&&>(v), doc.GetAllocator());
        }

        static  char*   itoa(int value)
        {
            static char tmp[1024];
            sprintf(tmp, "%d", value);
            return tmp;
        }
    };

    class FunctionMgr
    {
    public:
        void    execute(const char* str)
        {
            mDoc.Parse(str);

            string name = mDoc["name"].GetString();
            const Value& parmObject = mDoc["parm"];

            assert(mWrapFunctions.find(name) != mWrapFunctions.end());
            if (mWrapFunctions.find(name) != mWrapFunctions.end())
            {
                mWrapFunctions[name](mRealFunctionPtr[name], mDoc, parmObject);
            }
        }

        template<typename T>
        void insertLambda(string name, T lambdaObj)
        {
            _insertLambda<T>(name, lambdaObj, &T::operator());
        }

        template<typename ...Args>
        void insertStaticFunction(string name, void(*func)(Args...))
        {
            void* pbase = new VariadicArgFunctor<Args...>(func);

            mWrapFunctions[name] = VariadicArgFunctor<Args...>::invoke;
            mRealFunctionPtr[name] = pbase;
        }

        int     makeNextID()
        {
            mNextID++;
            return mNextID;
        }

        int     getNowID() const
        {
            return mNextID;
        }
    private:
        template<typename ...Args>
        struct VariadicArgFunctor
        {
            VariadicArgFunctor(std::function<void(Args...)> f)
            {
                mf = f;
            }

            static void invoke(void* pvoid, Document& doc, const Value& msg)
            {
                int parmIndex = 0;
                eval<Args...>(SizeType<sizeof...(Args)>::TYPE(), pvoid, doc, msg, parmIndex);
            }

            template<typename T, typename ...LeftArgs, typename ...NowArgs>
            static  void    eval(int _, void* pvoid, Document& doc, const Value& msg, int& parmIndex, NowArgs&... args)
            {
                remove_const<base_type<T>::type>::type cur_arg = Utils::readJsonByIndex<remove_const<base_type<T>::type>::type>(doc, msg, parmIndex++);
                eval<LeftArgs...>(SizeType<sizeof...(LeftArgs)>::TYPE(), pvoid, doc, msg, parmIndex, args..., cur_arg);
            }

            template<typename ...NowArgs>
            static  void    eval(char _, void* pvoid, Document& doc, const Value& msg, int& parmIndex, NowArgs&... args)
            {
                VariadicArgFunctor<Args...>* pthis = (VariadicArgFunctor<Args...>*)pvoid;
                (pthis->mf)(args...);
            }
        private:
            std::function<void(Args...)>   mf;
        };

        template<typename LAMBDA_OBJ_TYPE, typename ...Args>
        void _insertLambda(string name, LAMBDA_OBJ_TYPE obj, void(LAMBDA_OBJ_TYPE::*func)(Args...) const)
        {
            void* pbase = new VariadicArgFunctor<Args...>(obj);

            mWrapFunctions[name] = VariadicArgFunctor<Args...>::invoke;
            mRealFunctionPtr[name] = pbase;
        }

    private:
        typedef void(*pf_wrap)(void* pbase, Document& doc, const Value& msg);
        map<string, pf_wrap>        mWrapFunctions;
        map<string, void*>          mRealFunctionPtr;
        int                         mNextID;
        Document                    mDoc;
    };

    template<bool>
    struct SelectWriteArg;

    template<>
    struct SelectWriteArg<true>
    {
        template<typename ARGTYPE>
        static  void    Write(FunctionMgr& functionMgr, Document& doc, Value& parms, const ARGTYPE& arg, int index)
        {
            int id = functionMgr.makeNextID();
            functionMgr.insertLambda(Utils::itoa(id), arg);
        }
    };

    template<>
    struct SelectWriteArg<false>
    {
        template<typename ARGTYPE>
        static  void    Write(FunctionMgr& functionMgr, Document& doc, Value& parms, const ARGTYPE& arg, int index)
        {
            Utils::writeJsonByIndex(doc, parms, arg, index);
        }
    };

    class rpc
    {
    public:
        rpc() : mWriter(mBuffer)
        {
            /*  注册rpc_reply 服务函数，处理rpc返回值   */
            def("rpc_reply", [this](string response){
                handleResponse(response);
            });
        }

        template<typename F>
        void        def(const char* funname, F func)
        {
            regFunctor(funname, func);
        }

        /*  远程调用，返回值为经过序列化后的消息  */
        template<typename... Args>
        string    call(const char* funname, const Args&... args)
        {
            int old_req_id = mResponseCallbacks.getNowID();

            Value msg(kObjectType);
            msg.AddMember(GenericValue<UTF8<>>("name", mDoc.GetAllocator()), Value(funname, mDoc.GetAllocator()), mDoc.GetAllocator());
            int index = 0;
            
            Value parms(kObjectType);
            writeCallArg(mDoc, parms, index, args...);
            msg.AddMember(GenericValue<UTF8<>>("parm", mDoc.GetAllocator()), parms, mDoc.GetAllocator());

            int now_req_id = mResponseCallbacks.getNowID();
            /*req_id表示调用方的请求id，服务器(rpc被调用方)通过此id返回消息(返回值)给调用方*/
            msg.AddMember(GenericValue<UTF8<>>("req_id", mDoc.GetAllocator()), Value(old_req_id == now_req_id ? -1 : now_req_id), mDoc.GetAllocator());

            mBuffer.Clear();
            mWriter.Reset(mBuffer);
            msg.Accept(mWriter);
            return mBuffer.GetString();
        }

        /*  处理rpc请求 */
        void    handleRpc(string str)
        {
            mRpcFunctions.execute(str.c_str());
        }

        /*  返回数据给RPC调用端    */
        template<typename... Args>
        string    reply(int reqid, const Args&... args)
        {
            /*  把实际返回值打包作为参数,调用对端的rpc_reply 函数*/
            string response = call(Utils::itoa(reqid), args...);

            return call("rpc_reply", response);
        }

        /*  调用方处理收到的rpc返回值(消息)*/
        void    handleResponse(string str)
        {
            mResponseCallbacks.execute(str.c_str());
        }

    private:
        void    writeCallArg(Document& doc, int& index){}

        template<typename Arg>
        void    writeCallArg(Document& doc, Value& msg, int& index, const Arg& arg)
        {
            /*只(剩)有一个参数,肯定也为最后一个参数，允许为lambda*/
            _selectWriteArg(doc, msg, arg, index++);
        }

        template<typename Arg1, typename... Args>
        void    writeCallArg(Document& doc, Value& msg, int& index, const Arg1& arg1, const Args&... args)
        {
            Utils::writeJsonByIndex(doc, msg, arg1, index++);
            writeCallArg(doc, msg, index, args...);
        }
    private:

        /*如果是lambda则加入回调管理器，否则添加到rpc参数*/
        template<typename ARGTYPE>
        void    _selectWriteArg(Document& doc, Value& msg, const ARGTYPE& arg, int index)
        {
            SelectWriteArg<HasCallOperator<ARGTYPE>::value>::Write(mResponseCallbacks, doc, msg, arg, index);
        }

    private:
        template<typename ...Args>
        void regFunctor(const char* funname, void(*func)(Args...))
        {
            mRpcFunctions.insertStaticFunction(funname, func);
        }
        
        template<typename LAMBDA>
        void regFunctor(const char* funname, LAMBDA lambdaObj)
        {
            mRpcFunctions.insertLambda(funname, lambdaObj);
        }
    private:
        FunctionMgr                 mResponseCallbacks;
        FunctionMgr                 mRpcFunctions;
        Document                    mDoc;
        StringBuffer                mBuffer;
        Writer<StringBuffer>        mWriter;
    };
}

class Player : public dodo::rpc
{
public:
    Player()
    {
        registerHandle("player_attack", &Player::attack);
        registerHandle("player_hi", &Player::hi);
    }

private:
    template<typename... Args>
    void        registerHandle(string name, void (Player::*callback)(Args...))
    {
        def(name.c_str(), [this, callback](Args... args){
            (this->*callback)(args...);
        });
    }

private:
    void    attack(string target)
    {
        cout << "attack:" << target << endl;
    }

    void    hi(string i, string j)
    {
        cout << i << j << endl;
    }
};

void test1(int a, int b)
{
    cout << "in test1" << endl;
    cout << a << ", " << b << endl;
}

void test2(int a, int b, string c)
{
    cout << "in test2" << endl;
    cout << a << ", " << b << ", " << c << endl;
}

void test3(string a, int b, string c)
{
    cout << "in test3" << endl;
    cout << a << ", " << b << ", " << c << endl;
}

void test4(const string a, int b)
{
    cout << "in test4" << endl;
    cout << a << "," << b <<  endl;
}

void test5(const string a, int& b, const map<int, map<int, string>>& vlist)
{
}

void test6(string a, int b, map<string, int> vlist)
{
}

void test7(vector<map<int,string>>& vlist)
{
}
#ifdef _MSC_VER
#include <Windows.h>
#endif
int main()
{
    int upvalue = 10;
    using namespace dodo;

    Player rpc_server; /*rpc服务器*/
    Player rpc_client; /*rpc客户端*/

    string rpc_request_msg; /*  rpc消息   */
    string rpc_response_str;       /*  rpc返回值  */

    rpc_server.def("test4", test4);
    rpc_server.def("test5", test5);
    rpc_server.def("test7", test7);

    std::function<void(int)> functor = [](int i){
        cout << "i is " << i << endl;
    };
    rpc_server.def("test_functor", functor);
    rpc_server.def("test_lambda", [](int j){
        cout << "j is " << j << endl;
    });
    
    int count = 0;
#ifdef _MSC_VER
#include <Windows.h>
    DWORD starttime = GetTickCount();
    while (count++ <= 100000)
    {
        rpc_request_msg = rpc_client.call("test_functor", 1);
        rpc_server.handleRpc(rpc_request_msg);
    }

    cout << "cost :" << GetTickCount() - starttime << endl;
#endif
    

    rpc_request_msg = rpc_client.call("test_lambda", 2);
    rpc_server.handleRpc(rpc_request_msg);

    rpc_request_msg = rpc_client.call("player_attack", "Li Lei");
    rpc_server.handleRpc(rpc_request_msg);
    rpc_request_msg = rpc_client.call("player_hi", "Hello", "World");
    rpc_server.handleRpc(rpc_request_msg);
    
    {
        vector<map<int, string>> vlist;
        map<int, string> a = { { 1, "dzw" } };
        map<int, string> b = { { 2, "haha" } };
        vlist.push_back(a);
        vlist.push_back(b);

        int count = 0;
#ifdef _MSC_VER
#include <Windows.h>
        DWORD starttime = GetTickCount();
        while (count++ <= 100000)
        {
            rpc_request_msg = rpc_client.call("test7", vlist);
            rpc_server.handleRpc(rpc_request_msg);
        }

        cout << "cost :" << GetTickCount() - starttime << endl;
#endif
    }

    map<int, string> m1;
    m1[1] = "Li";
    map<int, string> m2;
    m2[2] = "Deng";
    map<int, map<int, string>> mlist;
    mlist[100] = m1;
    mlist[200] = m2;

    {
        rpc_request_msg = rpc_client.call("test5", "a", 1, mlist, [&upvalue](int a, int b){
            upvalue++;
            cout << "upvalue:" << upvalue << ", a:" << a << ", b:" << b << endl;
        });

        rpc_server.handleRpc(rpc_request_msg);
    }

    {
        rpc_request_msg = rpc_client.call("test5", "a", 1, mlist, [&upvalue](string a, string b, int c){
            upvalue++;
            cout << "upvalue:" << upvalue << ", a:" << a << ", b:" << b << ", c:" << c << endl;
        });

        rpc_server.handleRpc(rpc_request_msg);
    }

    {
        rpc_request_msg = rpc_client.call("test4", "a", 1);
        rpc_server.handleRpc(rpc_request_msg);
    }

    /*  模拟服务器通过reply返回数据给rpc client,然后rpc client处理收到的rpc返回值 */
    {
        rpc_response_str = rpc_server.reply(1, 1, 2);   /* (1,1,2)中的1为调用方的req_id, (1,2)为返回值 */
        rpc_client.handleRpc(rpc_response_str);
    }

    {
        rpc_response_str = rpc_server.reply(2, "hello", "world", 3);
        rpc_client.handleRpc(rpc_response_str);
    }

    cin.get();

    return 0;
}