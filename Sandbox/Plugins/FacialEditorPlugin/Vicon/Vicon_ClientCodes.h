// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef VICON_CLIENTCODES_H
#define VICON_CLIENTCODES_H

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || (DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT)
	#define DISABLE_VICON 1
#endif

typedef u32 EntityId; // Copied from IEntity.h

struct ICharacterInstance;

//////////////////////////////////////////////////////////////////////////
class ClientCodes
{
public:
	enum EType
	{
		ERequest,
		EReply
	};

	enum EPacket
	{
		EClose,
		EInfo,
		EData,
		EStreamOn,
		EStreamOff
	};

	static const std::vector<string> MarkerTokens;
	static const std::vector<string> BodyTokens;

	static std::vector<string> MakeMarkerTokens()
	{
		std::vector<string> v;
		v.push_back("<P-X>");
		v.push_back("<P-Y>");
		v.push_back("<P-Z>");
		v.push_back("<P-O>");
		return v;
	}

	static std::vector<string> MakeBodyTokens()
	{
		std::vector<string> v;
		v.push_back("<A-X>");
		v.push_back("<A-Y>");
		v.push_back("<A-Z>");
		v.push_back("<T-X>");
		v.push_back("<T-Y>");
		v.push_back("<T-Z>");
		return v;
	}

	struct CompareNames : std::binary_function<string, string, bool>
	{
		bool operator()(const string& a_S1, const string& a_S2) const
		{
			string::const_iterator iS1 = a_S1.begin();
			string::const_iterator iS2 = a_S2.begin();

			while (iS1 != a_S1.end() && iS2 != a_S2.end())
				if (toupper(*(iS1++)) != toupper(*(iS2++))) return false;

			return a_S1.size() == a_S2.size();
		}
	};
};

//////////////////////////////////////////////////////////////////////////
class MarkerChannel
{
public:
	string Name;

	i32    X;
	i32    Y;
	i32    Z;
	i32    O;

	MarkerChannel(string& a_rName) : X(-1), Y(-1), Z(-1), O(-1), Name(a_rName) {}

	i32& operator[](i32 i)
	{
		switch (i)
		{
		case 0:
			return X;
		case 1:
			return Y;
		case 2:
			return Z;
		case 3:
			return O;
		default:
			assert(false);
			return O;
		}
	}

	i32 operator[](i32 i) const
	{
		switch (i)
		{
		case 0:
			return X;
		case 1:
			return Y;
		case 2:
			return Z;
		case 3:
			return O;
		default:
			assert(false);
			return -1;
		}
	}

	bool operator==(const string& a_rName)
	{
		ClientCodes::CompareNames comparitor;
		return comparitor(Name, a_rName);
	}
};

//////////////////////////////////////////////////////////////////////////
class MarkerData
{
public:
	double X;
	double Y;
	double Z;
	bool   Visible;

	Vec3 TransformData()
	{
		Vec3 vPos(X, Z, Y);
		vPos /= 1000.0f;
		return (vPos);
	}
};

class BodyChannel
{
public:
	string Name;

	string m_BipName;
	f32    m_x;
	f32    m_y;
	f32    m_z;

	i32    TX;
	i32    TY;
	i32    TZ;
	i32    RX;
	i32    RY;
	i32    RZ;

	BodyChannel(string& a_rName) : RX(-1), RY(-1), RZ(-1), TX(-1), TY(-1), TZ(-1), Name(a_rName) {}

	i32& operator[](i32 i)
	{
		switch (i)
		{
		case 0:
			return RX;
		case 1:
			return RY;
		case 2:
			return RZ;
		case 3:
			return TX;
		case 4:
			return TY;
		case 5:
			return TZ;
		default:
			assert(false);
			return TZ;
		}
	}

	i32 operator[](i32 i) const
	{
		switch (i)
		{
		case 0:
			return RX;
		case 1:
			return RY;
		case 2:
			return RZ;
		case 3:
			return TX;
		case 4:
			return TY;
		case 5:
			return TZ;
		default:
			assert(false);
			return -1;
		}
	}

	bool operator==(const string& a_rName)
	{
		ClientCodes::CompareNames comparitor;
		return comparitor(Name, a_rName);
	}
};

//////////////////////////////////////////////////////////////////////////
class BodyData
{
public:
	// Representation of body translation
	//double	TX;
	//double	TY;
	//double	TZ;
	u32 m_IsInitialized;
	QuatTd Joint;

	// Representation of body rotation Quaternion
	//double	QX;
	//double	QY;
	//double	QZ;
	//double	QW;
	// Global rotation matrix
	//double GlobalRotation[3][3];

	//double EulerX;
	//double EulerY;
	//double EulerZ;

	ILINE BodyData()
	{
		m_IsInitialized = 0;
		//Joint.SetIdentity();
		//QW=1.0f;
		//QX=0.0f;
		//QY=0.0f;
		//QZ=0.0f;
	};

};

//////////////////////////////////////////////////////////////////////////
struct tNetError
{
	u32      nrErrorCode;
	tukk sErrorDescription;
};

//////////////////////////////////////////////////////////////////////////
class CViconClient
{
public:
	CViconClient();
	~CViconClient();

	bool        Connect(IConsoleCmdArgs* pArgs);
	void        Disconnect();
	void        Update();
	tukk GetErrorDescription(u32 nError);
	void        PrintErrorDescription(u32 nError);

	void        ExternalPostProcessing(ICharacterInstance* pInstance);
	i32       GetIDbyName(tukk strJointName);
	void        FixInverseBending(i32 Pelvis, i32 Thigh, i32 Calf, i32 Foot, i32 Toe0);
	void        TwoBoneSolver(Vec3d goal, i32 Pelvis, i32 Thigh, i32 Calf, i32 Foot, i32 Toe0, i32 Toe1);

	std::vector<MarkerChannel> m_MarkerChannels;
	std::vector<BodyChannel>   m_BodyChannels;

	std::vector<MarkerData>    m_markerPositions;
	std::vector<BodyData>      m_bodyPositions;

	std::vector<double>        m_data;
	std::vector<string>        m_info;

	i32                        m_nFrameChannel;
	i32                        m_nFrameCounter;
	Vec3                       vMid;

	bool                       m_bConnected;

	std::vector<string>        m_lstEntityNames;
	std::vector<EntityId>      m_lstEntities;
	std::vector<Vec3>          m_lstEntitiesOriginPos;
	std::vector<Quat>          m_lstEntitiesOriginRot;
	//IEntity *m_pEntities[VICON_MAX_ACTORS];
	//IEntity *m_pEntitiesProps[VICON_MAX_PROPS];
	//Vec3 m_vEntitiesProps[VICON_MAX_PROPS];

	i32   m_SocketHandle;

	float m_fTimeStamp, m_fLastTimeStamp;

private:

	MarkerData* FindMarker(tukk szName);
	Vec3        GetTransformedMarkerPos(tukk szName);
	Plane       FitMarkers(bool bUseDefault);

};

#endif

