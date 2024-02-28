#include <GarrysMod/FactoryLoader.hpp>
#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/Lua/LuaShared.h>
#include <lua.h>

#define LUA_SHARED_INTERFACE_VERSION "LUASHARED003"

#define COPY_STATE_TYPE(type)							\
	case GarrysMod::Lua::Type::##type:					\
		LUA->Push##type(LUA_CL->Get##type(iStackPos));	\
		break;

enum
{
	LUA_CLIENT = 0,
	LUA_SERVER,
	LUA_MENU
};

GarrysMod::Lua::ILuaBase* LUA_CL;

bool pushnetfunction(const char* name)
{
	LUA_CL->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	{
		// Get _G.net
		LUA_CL->PushString("net");
		LUA_CL->RawGet(-2);

		if (LUA_CL->IsType(-1, GarrysMod::Lua::Type::Table))
		{
			// Get _G.net[name]
			LUA_CL->PushString(name);
			LUA_CL->RawGet(-2);

			if (LUA_CL->IsType(-1, GarrysMod::Lua::Type::Function)) // -1 is now the function
			{
				return true;
			}
			LUA_CL->Pop();
		}
		LUA_CL->Pop();
	}
	LUA_CL->Pop();

	// Something failed
	return false;
}

void callnetfunction(int iArgs, int iResults)
{
	if (LUA_CL->PCall(iArgs, iResults, 0) != LUA_OK)
	{
		LUA_CL->ThrowError(LUA_CL->GetString(-1));
		LUA_CL->Pop();
	}
}

void cleanupnetfunction(int iKeep)
{
	int iCount = LUA_CL->Top();
	if (iCount <= iKeep) return;

	if (iKeep == 0)
	{
		// Pop everything if we're not keeping it
		LUA_CL->Pop(iCount);
		return;
	}

	int iRemoveTimes = iCount - iKeep;
	int iRemoveIndex = (iCount - iRemoveTimes) + 1;

	// Pop all the stuff we don't want
	for (int i = 0; i < iRemoveTimes; i++)
		LUA_CL->Remove(iRemoveIndex);
}

int copyreturns(GarrysMod::Lua::ILuaBase* LUA)
{
	int iCount = LUA_CL->Top();
	if (iCount == 0) return 0; // Nothing to copy

	// Copy the stack from one Lua state to another
	for (int i = iCount; i > 0; i--)
	{
		int iStackPos = -i;
		int iType = LUA_CL->GetType(iStackPos);

		// Ugh
		switch (iType)
		{
		default:
		case GarrysMod::Lua::Type::Nil:
			LUA->PushNil();
			break;

			COPY_STATE_TYPE(Angle)
			COPY_STATE_TYPE(Bool)
			COPY_STATE_TYPE(Number)
			COPY_STATE_TYPE(String)
			COPY_STATE_TYPE(Vector)
		}
	}

	// Remove from the client state
	LUA_CL->Pop(iCount);

	return iCount;
}

void pushcfunction(GarrysMod::Lua::ILuaBase* LUA, GarrysMod::Lua::CFunc function, const char* name)
{
	LUA->PushString(name);
	LUA->PushCFunction(function);
	LUA->RawSet(-3);
}

LUA_FUNCTION(Start)
{
	const char* szMessageName = LUA->CheckString(1);

	bool bUnreliable;
	if (LUA->IsType(2, GarrysMod::Lua::Type::Bool))
		bUnreliable = LUA->GetBool(2);
	else
		bUnreliable = false;

	// Push net.Start
	if (!pushnetfunction("Start"))
	{
		LUA->PushBool(false);
		return 1;
	}

	// Give the arguments
	LUA_CL->PushString(szMessageName);
	LUA_CL->PushBool(bUnreliable);

	// Call it
	callnetfunction(2, 1);

	// Keep its return(s)
	cleanupnetfunction(1);

	// Copy the return(s) over
	return copyreturns(LUA);
}

LUA_FUNCTION(Abort)
{
	if (!pushnetfunction("Abort")) return 0;
	callnetfunction(0, 0);
	cleanupnetfunction(0);
	return copyreturns(LUA);
}

LUA_FUNCTION(BytesLeft)
{
	if (!pushnetfunction("BytesLeft"))
	{
		LUA->PushNil();
		LUA->PushNil();
		return 2;
	}

	callnetfunction(0, 2);
	cleanupnetfunction(2);
	return copyreturns(LUA);
}

LUA_FUNCTION(BytesWritten)
{
	if (!pushnetfunction("BytesWritten"))
	{
		LUA->PushNil();
		LUA->PushNil();
		return 2;
	}

	callnetfunction(0, 2);
	cleanupnetfunction(2);
	return copyreturns(LUA);
}

LUA_FUNCTION(ReadAngle)
{
	if (!pushnetfunction("ReadAngle"))
	{
		LUA->PushAngle(QAngle(0, 0, 0));
		return 1;
	}

	callnetfunction(0, 1);
	cleanupnetfunction(1);
	return copyreturns(LUA);
}

LUA_FUNCTION(ReadBit)
{
	if (!pushnetfunction("ReadBit"))
	{
		LUA->PushNumber(0);
		return 1;
	}

	callnetfunction(0, 1);
	cleanupnetfunction(1);
	return copyreturns(LUA);
}

LUA_FUNCTION(ReadBool)
{
	if (!pushnetfunction("ReadBool"))
	{
		LUA->PushBool(false);
		return 1;
	}

	callnetfunction(0, 1);
	cleanupnetfunction(1);
	return copyreturns(LUA);
}

// Read Color

LUA_FUNCTION(ReadData)
{
	int iLength = LUA->CheckNumber(1);

	if (!pushnetfunction("ReadData"))
	{
		LUA->PushString("");
		return 1;
	}

	LUA_CL->PushNumber(iLength);
	callnetfunction(1, 1);

	cleanupnetfunction(1);
	return copyreturns(LUA);
}

LUA_FUNCTION(ReadDouble)
{
	if (!pushnetfunction("ReadDouble"))
	{
		LUA->PushNumber(0);
		return 1;
	}

	callnetfunction(0, 1);
	cleanupnetfunction(1);
	return copyreturns(LUA);
}

// Read Entity

LUA_FUNCTION(ReadFloat)
{
	if (!pushnetfunction("ReadFloat"))
	{
		LUA->PushNumber(0);
		return 1;
	}

	callnetfunction(0, 1);
	cleanupnetfunction(1);
	return copyreturns(LUA);
}

LUA_FUNCTION(ReadInt)
{
	int iBitCount = LUA->CheckNumber(1);

	if (!pushnetfunction("ReadInt"))
	{
		LUA->PushNumber(0);
		return 1;
	}

	LUA_CL->PushNumber(iBitCount);
	callnetfunction(1, 1);

	cleanupnetfunction(1);
	return copyreturns(LUA);
}

// Read Matrix

LUA_FUNCTION(ReadNormal)
{
	if (!pushnetfunction("ReadNormal"))
	{
		LUA->PushVector(Vector(0, 0, 1));
		return 1;
	}

	callnetfunction(0, 1);
	cleanupnetfunction(1);
	return copyreturns(LUA);
}

// Read Player

LUA_FUNCTION(ReadString)
{
	if (!pushnetfunction("ReadString"))
	{
		LUA->PushString("");
		return 1;
	}

	callnetfunction(0, 1);
	cleanupnetfunction(1);
	return copyreturns(LUA);
}

// Read Table

LUA_FUNCTION(ReadUInt)
{
	int iBitCount = LUA->CheckNumber(1);

	if (!pushnetfunction("ReadUInt"))
	{
		LUA->PushNumber(0);
		return 1;
	}

	LUA_CL->PushNumber(iBitCount);
	callnetfunction(1, 1);

	cleanupnetfunction(1);
	return copyreturns(LUA);
}

LUA_FUNCTION(ReadUInt64)
{
	if (!pushnetfunction("ReadUInt64"))
	{
		LUA->PushString("0");
		return 1;
	}

	callnetfunction(0, 1);
	cleanupnetfunction(1);
	return copyreturns(LUA);
}

LUA_FUNCTION(ReadVector)
{
	if (!pushnetfunction("ReadVector"))
	{
		LUA->PushVector(Vector(0, 0, 0));
		return 1;
	}

	callnetfunction(0, 1);
	cleanupnetfunction(1);
	return copyreturns(LUA);
}

// Receive

LUA_FUNCTION(WriteAngle)
{
	LUA->CheckType(1, GarrysMod::Lua::Type::Angle);
	QAngle angle = LUA->GetAngle(1);

	if (!pushnetfunction("WriteAngle")) return 0;

	LUA_CL->PushAngle(angle);
	callnetfunction(1, 0);

	cleanupnetfunction(0);
	return copyreturns(LUA);
}

LUA_FUNCTION(WriteBit)
{
	LUA->CheckType(1, GarrysMod::Lua::Type::Bool);
	bool bBit = LUA->GetBool(1);

	if (!pushnetfunction("WriteBit")) return 0;

	LUA_CL->PushBool(bBit);
	callnetfunction(1, 0);

	cleanupnetfunction(0);
	return copyreturns(LUA);
}

LUA_FUNCTION(WriteBool)
{
	LUA->CheckType(1, GarrysMod::Lua::Type::Bool);
	bool bBit = LUA->GetBool(1);

	if (!pushnetfunction("WriteBool")) return 0;

	LUA_CL->PushBool(bBit);
	callnetfunction(1, 0);

	cleanupnetfunction(0);
	return copyreturns(LUA);
}

// Write Color

LUA_FUNCTION(WriteData)
{
	const char* szData = LUA->CheckString(1);

	int iLength;
	if (LUA->IsType(2, GarrysMod::Lua::Type::Number))
		iLength = LUA->GetNumber(2);
	else
		iLength = strlen(szData);

	if (!pushnetfunction("WriteData")) return 0;

	LUA_CL->PushString(szData);
	LUA_CL->PushNumber(iLength);
	callnetfunction(2, 0);

	cleanupnetfunction(0);
	return copyreturns(LUA);
}

LUA_FUNCTION(WriteDouble)
{
	double dbAmount = LUA->CheckNumber(1);

	if (!pushnetfunction("WriteDouble")) return 0;

	LUA_CL->PushNumber(dbAmount);
	callnetfunction(1, 0);

	cleanupnetfunction(0);
	return copyreturns(LUA);
}

// Write Entity

LUA_FUNCTION(WriteFloat)
{
	float flAmount = LUA->CheckNumber(1);

	if (!pushnetfunction("WriteFloat")) return 0;

	LUA_CL->PushNumber(flAmount);
	callnetfunction(1, 0);

	cleanupnetfunction(0);
	return copyreturns(LUA);
}

LUA_FUNCTION(WriteInt)
{
	int iNumber = LUA->CheckNumber(1);
	int iBitCount = LUA->CheckNumber(2);

	if (!pushnetfunction("WriteInt")) return 0;

	LUA_CL->PushNumber(iNumber);
	LUA_CL->PushNumber(iBitCount);
	callnetfunction(2, 0);

	cleanupnetfunction(0);
	return copyreturns(LUA);
}

// Write Matrix

LUA_FUNCTION(WriteNormal)
{
	LUA->CheckType(1, GarrysMod::Lua::Type::Vector);
	Vector vector = LUA->GetVector(1);

	if (!pushnetfunction("WriteNormal")) return 0;

	LUA_CL->PushVector(vector);
	callnetfunction(1, 0);

	cleanupnetfunction(0);
	return copyreturns(LUA);
}

// Write Player

LUA_FUNCTION(WriteString)
{
	const char* szString = LUA->CheckString(1);

	if (!pushnetfunction("WriteString")) return 0;

	LUA_CL->PushString(szString);
	callnetfunction(1, 0);

	cleanupnetfunction(0);
	return copyreturns(LUA);
}

// Write Table

LUA_FUNCTION(WriteUInt)
{
	int iNumber = LUA->CheckNumber(1);
	int iBitCount = LUA->CheckNumber(2);

	if (!pushnetfunction("WriteUInt")) return 0;

	LUA_CL->PushNumber(iNumber);
	LUA_CL->PushNumber(iBitCount);
	callnetfunction(2, 0);

	cleanupnetfunction(0);
	return copyreturns(LUA);
}

LUA_FUNCTION(WriteUInt64)
{
	const char* szString = LUA->CheckString(1);

	if (!pushnetfunction("WriteUInt64")) return 0;

	LUA_CL->PushString(szString);
	callnetfunction(1, 0);

	cleanupnetfunction(0);
	return copyreturns(LUA);
}

LUA_FUNCTION(WriteVector)
{
	LUA->CheckType(1, GarrysMod::Lua::Type::Vector);
	Vector vector = LUA->GetVector(1);

	if (!pushnetfunction("WriteVector")) return 0;

	LUA_CL->PushVector(vector);
	callnetfunction(1, 0);

	cleanupnetfunction(0);
	return copyreturns(LUA);
}

LUA_FUNCTION(SendToServer)
{
	if (!pushnetfunction("SendToServer")) return 0;
	callnetfunction(0, 0);
	cleanupnetfunction(0);
	return copyreturns(LUA);
}

GMOD_MODULE_OPEN()
{
	// Get Lua shared
	SourceSDK::FactoryLoader factory_LuaShared("lua_shared");
	GarrysMod::Lua::ILuaShared* pLuaShared = factory_LuaShared.GetInterface<GarrysMod::Lua::ILuaShared>(LUA_SHARED_INTERFACE_VERSION);
	if (!pLuaShared)
	{
		LUA->ThrowError("Can't find Lua Shared");
		return 0;
	}

	// Get the clientside Lua state
	LUA_CL = (GarrysMod::Lua::ILuaBase*)pLuaShared->GetLuaInterface(LUA_CLIENT);
	if (!LUA_CL)
	{
		// What are we running as??
		LUA->ThrowError("No ClientSide Lua state (?)");
		return 0;
	}

	// Push the stuff
	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	{
		LUA->CreateTable();
		{
			pushcfunction(LUA, Start, "Start");
			pushcfunction(LUA, Abort, "Abort");
			pushcfunction(LUA, BytesLeft, "BytesLeft");
			pushcfunction(LUA, BytesWritten, "BytesWritten");
			pushcfunction(LUA, ReadAngle, "ReadAngle");
			pushcfunction(LUA, ReadBit, "ReadBit");
			pushcfunction(LUA, ReadBool, "ReadBool");
			// Read Color
			pushcfunction(LUA, ReadData, "ReadData");
			pushcfunction(LUA, ReadDouble, "ReadDouble");
			// Read Entity
			pushcfunction(LUA, ReadFloat, "ReadFloat");
			pushcfunction(LUA, ReadInt, "ReadInt");
			// Read Matrix
			pushcfunction(LUA, ReadNormal, "ReadNormal");
			// Read Player
			pushcfunction(LUA, ReadString, "ReadString");
			// Read Table
			pushcfunction(LUA, ReadUInt, "ReadUInt");
			pushcfunction(LUA, ReadUInt64, "ReadUInt64");
			pushcfunction(LUA, ReadVector, "ReadVector");
			// Receive
			pushcfunction(LUA, WriteAngle, "WriteAngle");
			pushcfunction(LUA, WriteBit, "WriteBit");
			pushcfunction(LUA, WriteBool, "WriteBool");
			// Write Color
			pushcfunction(LUA, WriteData, "WriteData");
			// Write Entity
			pushcfunction(LUA, WriteFloat, "WriteFloat");
			pushcfunction(LUA, WriteInt, "WriteInt");
			// Write Matrix
			pushcfunction(LUA, WriteNormal , "WriteNormal");
			// Write Player
			pushcfunction(LUA, WriteString, "WriteString");
			// Write Table
			pushcfunction(LUA, WriteUInt, "WriteUInt");
			pushcfunction(LUA, WriteUInt64, "WriteUInt64");
			pushcfunction(LUA, WriteVector, "WriteVector");
			pushcfunction(LUA, SendToServer, "SendToServer");
		}
		LUA->PushString("net");
		LUA->Push(-2);
		LUA->RawSet(-4);
	}
	LUA->Pop();

	return 0;
}

GMOD_MODULE_CLOSE()
{
	return 0;
}