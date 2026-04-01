/******************************************************** 
* author: scofieldzhu
* time:2025/1/25
*******************************************************/
#ifndef __mobject_h__
#define __mobject_h__

#include "helios/helios_nsp.h"
#include <cstring>

HELIOS_NAMESPACE_BEGIN

class MObject
{
public:
	virtual const char* getClsTypeName() const { return GetClsTypeName(); }
	virtual bool isA(const char* type)const { return MObject::IsTypeOf(type); }
	static bool IsTypeOf(const char* name) { return std::strcmp(GetClsTypeName(), name) == 0; }
	static const char* GetClsTypeName() { return "MObject"; }
	virtual ~MObject() = default;

protected:
	MObject() = default;
};

#define MOBJECT_DECL(TheClass, SuperClass, RootClass) \
    public:\
        using __super__ = SuperClass; \
        virtual const char* getClsTypeName() const { return #TheClass; }\
        static const char* GetClsTypeName(){ return #TheClass; }\
        static bool IsTypeOf(const char* type){\
            if(!strcmp(#TheClass, type)) return true;\
            return SuperClass::IsTypeOf(type);\
        }\
        virtual bool isA(const char* type)const{ return this->TheClass::IsTypeOf(type);}\
        static TheClass* SafeDownCast(RootClass* e){ \
            if(e && e->isA(#TheClass)) return static_cast<TheClass*>(e); \
            return nullptr;\
        }\
        static const TheClass* SafeDownCast(const RootClass* e){\
            if(e && e->isA(#TheClass)) return static_cast<const TheClass*>(e); \
            return nullptr; \
        }

#define SPRITE_DECL(TheClass, SuperClass) MOBJECT_DECL(TheClass, SuperClass, Sprite)

#define SCENE_DECL(TheClass, SuperClass) MOBJECT_DECL(TheClass, SuperClass, Scene)

NAMESPACE_END

#endif