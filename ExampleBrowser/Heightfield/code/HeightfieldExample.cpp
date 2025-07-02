#include "../HeightfieldExample.h"		// always include our own header first!

#include <drx3D/DynamicsCommon.h>
#include <drx3D/Physics/Collision/Shapes/HeightfieldTerrainShape.h>
#include <drx3D/Common/Interfaces/CommonRigidBodyBase.h>
#include "../../MultiThreadedDemo/CommonRigidBodyMTBase.h"
#include <drx3D/Common/Interfaces/CommonParameterInterface.h>
#include <drx3D/OpenGLWindow/GLInstanceGraphicsShape.h>
#include <drx3D/Common/DefaultFileIO.h>
#include <drx3D/Importers/URDF/urdfStringSplit.h>
#include <X/stb/stb_image.h>

// constants -------------------------------------------------------------------
static const Scalar s_gravity = 9.8;		// 9.8 m/s^2

static i32 s_gridSize = 16 + 1;  // must be (2^N) + 1
static Scalar s_gridSpacing = 0.5;
static Scalar s_gridHeightScale = 0.02;

// the singularity at the center of the radial model means we need a lot of
//   finely-spaced time steps to get the physics right.
// These numbers are probably too aggressive for a real game!

// delta phase: radians per second
static const Scalar s_deltaPhase = 0.25 * 2.0 * SIMD_PI;

// what type of terrain is generated?
enum eTerrainModel {
	eRadial = 0,	// deterministic
	eFractal = 1,	// random
	eCSVFile = 2,//csv file used in DeepLoco for example
	eImageFile = 3,//terrain from png/jpg files, asset from https://www.beamng.com/threads/tutorial-adding-heightmap-roads-using-blender.16356/

};


//typedef u8 byte_t;



////////////////////////////////////////////////////////////////////////////////
//
//	static helper methods
//
//	Only used within this file (helpers and terrain generation, etc)
//
////////////////////////////////////////////////////////////////////////////////

static tukk getTerrainTypeName(eTerrainModel model)
{
	switch (model) {
	case eRadial:
		return "Radial";

	case eFractal:
		return "Fractal";
    case eCSVFile:
        return "DeepLocoCSV";
    case eImageFile:
        return "Image";
	default:
		Assert(!"bad terrain model type");
	}

	return nullptr;
}






static Vec3
getUpVector
(
	i32 upAxis,
	Scalar regularValue,
	Scalar upValue
)
{
	Assert(upAxis >= 0 && upAxis <= 2 && "bad up axis");

	Vec3 v(regularValue, regularValue, regularValue);
	v[upAxis] = upValue;

	return v;
}



// TODO: it would probably cleaner to have a struct per data type, so
// 	you could lookup byte sizes, conversion functions, etc.
static i32 getByteSize
(
	PHY_ScalarType type
)
{
	i32 size = 0;

	switch (type) {
	case PHY_FLOAT:
		size = sizeof(Scalar);
		break;

	case PHY_UCHAR:
		size = sizeof(u8);
		break;

	case PHY_SHORT:
		size = sizeof(short);
		break;

	default:
		Assert(!"Bad heightfield data type");
	}

	return size;
}



static Scalar
convertToFloat
(
	const byte_t * p,
	PHY_ScalarType type
)
{
	Assert(p);

	switch (type) {
	case PHY_FLOAT:
	{
		Scalar * pf = (Scalar *)p;
		return *pf;
	}

	case PHY_UCHAR:
	{
		u8 * pu = (u8*)p;
		return ((*pu) * s_gridHeightScale);
	}

	case PHY_SHORT:
	{
		short * ps = (short *)p;
		return ((*ps) * s_gridHeightScale);
	}

	default:
		Assert(!"bad type");
	}

	return 0;
}



static Scalar
getGridHeight
(
	byte_t * grid,
	i32 i,
	i32 j,
	PHY_ScalarType type
)
{
	Assert(grid);
	Assert(i >= 0 && i < s_gridSize);
	Assert(j >= 0 && j < s_gridSize);

	i32 bpe = getByteSize(type);
	Assert(bpe > 0 && "bad bytes per element");

	i32 idx = (j * s_gridSize) + i;
	long offset = ((long)bpe) * idx;

	byte_t * p = grid + offset;

	return convertToFloat(p, type);
}



static void
convertFromFloat
(
	byte_t * p,
	Scalar value,
	PHY_ScalarType type
)
{
	Assert(p && "null");

	switch (type) {
	case PHY_FLOAT:
	{
		Scalar * pf = (Scalar *)p;
		*pf = value;
	}
	break;

	case PHY_UCHAR:
	{
		u8 * pu = (u8*)p;
		*pu = (u8)(value / s_gridHeightScale);
	}
	break;

	case PHY_SHORT:
	{
		short * ps = (short *)p;
		*ps = (short)(value / s_gridHeightScale);
	}
	break;

	default:
		Assert(!"bad type");
	}
}



// creates a radially-varying heightfield
static void
setRadial
(
	byte_t * grid,
	i32 bytesPerElement,
	PHY_ScalarType type,
	Scalar phase = 0.0
)
{
	Assert(grid);
	Assert(bytesPerElement > 0);

	// min/max
	Scalar period = 0.5 / s_gridSpacing;
	Scalar floor = 0.0;
	Scalar min_r = 3.0 * Sqrt(s_gridSpacing);
	Scalar magnitude = 5.0 * Sqrt(s_gridSpacing);

	// pick a base_phase such that phase = 0 results in max height
	//   (this way, if you create a heightfield with phase = 0,
	//    you can rely on the min/max heights that result)
	Scalar base_phase = (0.5 * SIMD_PI) - (period * min_r);
	phase += base_phase;

	// center of grid
	Scalar cx = 0.5 * s_gridSize * s_gridSpacing;
	Scalar cy = cx;		// assume square grid
	byte_t * p = grid;
	for (i32 i = 0; i < s_gridSize; ++i) {
		float x = i * s_gridSpacing;
		for (i32 j = 0; j < s_gridSize; ++j) {
			float y = j * s_gridSpacing;

			float dx = x - cx;
			float dy = y - cy;

			float r = sqrt((dx * dx) + (dy * dy));

			float z = period;
			if (r < min_r) {
				r = min_r;
			}
			z = (1.0 / r) * sin(period * r + phase);
			if (z > period) {
				z = period;
			}
			else if (z < -period) {
				z = -period;
			}
			z = floor + magnitude * z;

			convertFromFloat(p, z, type);
			p += bytesPerElement;
		}
	}
}



static float
randomHeight
(
	i32 step
)
{
	return (0.33 * s_gridSpacing * s_gridSize * step * (rand() - (0.5 * RAND_MAX))) / (1.0 * RAND_MAX * s_gridSize);
}


#if 0
static void
dumpGrid
(
	const byte_t * grid,
	i32 bytesPerElement,
	PHY_ScalarType type,
	i32 max
)
{
	//std::cerr << "Grid:\n";

	char buffer[32];

	for (i32 j = 0; j < max; ++j) {
		for (i32 i = 0; i < max; ++i) {
			long offset = j * s_gridSize + i;
			float z = convertToFloat(grid + offset * bytesPerElement, type);
			sprintf(buffer, "%6.2f", z);
			//std::cerr << "  " << buffer;
		}
		//std::cerr << "\n";
	}
}
#endif


static void
updateHeight
(
	byte_t * p,
	Scalar new_val,
	PHY_ScalarType type
)
{
	Scalar old_val = convertToFloat(p, type);
	if (!old_val) {
		convertFromFloat(p, new_val, type);
	}
}



// creates a random, fractal heightfield
static void
setFractal
(
	byte_t * grid,
	i32 bytesPerElement,
	PHY_ScalarType type,
	i32 step
)
{
	Assert(grid);
	Assert(bytesPerElement > 0);
	Assert(step > 0);
	Assert(step < s_gridSize);

	i32 newStep = step / 2;
	//	std::cerr << "Computing grid with step = " << step << ": before\n";
	//	dumpGrid(grid, bytesPerElement, type, step + 1);

	// special case: starting (must set four corners)
	if (s_gridSize - 1 == step) {
		// pick a non-zero (possibly negative) base elevation for testing
		Scalar base = randomHeight(step / 2);

		convertFromFloat(grid, base, type);
		convertFromFloat(grid + step * bytesPerElement, base, type);
		convertFromFloat(grid + step * s_gridSize * bytesPerElement, base, type);
		convertFromFloat(grid + (step * s_gridSize + step) * bytesPerElement, base, type);
	}

	// determine elevation of each corner
	Scalar c00 = convertToFloat(grid, type);
	Scalar c01 = convertToFloat(grid + step * bytesPerElement, type);
	Scalar c10 = convertToFloat(grid + (step * s_gridSize) * bytesPerElement, type);
	Scalar c11 = convertToFloat(grid + (step * s_gridSize + step) * bytesPerElement, type);

	// set top middle
	updateHeight(grid + newStep * bytesPerElement, 0.5 * (c00 + c01) + randomHeight(step), type);

	// set left middle
	updateHeight(grid + (newStep * s_gridSize) * bytesPerElement, 0.5 * (c00 + c10) + randomHeight(step), type);

	// set right middle
	updateHeight(grid + (newStep * s_gridSize + step) * bytesPerElement, 0.5 * (c01 + c11) + randomHeight(step), type);

	// set bottom middle
	updateHeight(grid + (step * s_gridSize + newStep) * bytesPerElement, 0.5 * (c10 + c11) + randomHeight(step), type);

	// set middle
	updateHeight(grid + (newStep * s_gridSize + newStep) * bytesPerElement, 0.25 * (c00 + c01 + c10 + c11) + randomHeight(step), type);

	//	std::cerr << "Computing grid with step = " << step << ": after\n";
	//	dumpGrid(grid, bytesPerElement, type, step + 1);

	// terminate?
	if (newStep < 2) {
		return;
	}

	// recurse
	setFractal(grid, bytesPerElement, type, newStep);
	setFractal(grid + newStep * bytesPerElement, bytesPerElement, type, newStep);
	setFractal(grid + (newStep * s_gridSize) * bytesPerElement, bytesPerElement, type, newStep);
	setFractal(grid + ((newStep * s_gridSize) + newStep) * bytesPerElement, bytesPerElement, type, newStep);
}


#define MYLINELENGTH 16*32768

static byte_t *
getRawHeightfieldData
(
	eTerrainModel model,
	PHY_ScalarType type,
	Scalar& minHeight,
	Scalar& maxHeight
)
{

    if (model==eImageFile)
    {

        DefaultFileIO fileIO;
        char relativeFileName[1024];
        i32 found = fileIO.findFile("heightmaps/wm_height_out.png", relativeFileName, 1024);
        

        b3AlignedObjectArray<char> buffer;
        buffer.reserve(1024);
        i32 fileId = fileIO.fileOpen(relativeFileName,"rb");
        if (fileId>=0)
        {
            i32 size = fileIO.getFileSize(fileId);
            if (size>0)
            {
                buffer.resize(size);
                i32 actual = fileIO.fileRead(fileId,&buffer[0],size);
                if (actual != size)
                {
                    drx3DWarning("Насовпадение размера файла STL!\n");
                    buffer.resize(0);
                }
            }
            fileIO.fileClose(fileId);
        }

        if (buffer.size())
        {
            i32 width, height,n;

            u8* image = stbi_load_from_memory((u8k*)&buffer[0], buffer.size(), &width, &height, &n, 3);
            if (image)
            {
                printf("width=%d, height=%d at %d channels\n", width,height, n);
                s_gridSize = width;
                s_gridSpacing = 0.2;
                s_gridHeightScale = 0.2;
                fileIO.fileClose(fileId);
                long nElements = ((long)s_gridSize) * s_gridSize;
                //	std::cerr << "  nElements = " << nElements << "\n";

                i32 bytesPerElement = getByteSize(type);
                //	std::cerr << "  bytesPerElement = " << bytesPerElement << "\n";
                Assert(bytesPerElement > 0 && "bad bytes per element");

                long nBytes = nElements * bytesPerElement;
                //	std::cerr << "  nBytes = " << nBytes << "\n";
                byte_t * raw = new byte_t[nBytes];
                Assert(raw && "out of memory");

                byte_t * p = raw;
                
				for (i32 j = 0; j < width; ++j)
                {
                    
					for (i32 i = 0; i < width; ++i)
                    {
						float x = i * s_gridSpacing;
                        float y = j * s_gridSpacing;
						float heightScaling = (14. / 256.);
						float z = double(image[(width - 1 - i) * 3 + width*j * 3]) * heightScaling;
                        convertFromFloat(p, z, type);
						// update min/max
						if (!i && !j) {
							minHeight = z;
							maxHeight = z;
						}
						else {
							if (z < minHeight) {
								minHeight = z;
							}
							if (z > maxHeight) {
								maxHeight = z;
							}
						}

                        p += bytesPerElement;
                    }
                }
                return raw;

            }

        }





    }

    if (model==eCSVFile)
    {
        {
            DefaultFileIO fileIO;
            char relativePath[1024];
            i32 found = fileIO.findFile("heightmaps/ground0.txt", relativePath, 1024);
            char lineBuffer[MYLINELENGTH];
            i32 slot = fileIO.fileOpen(relativePath, "r");
            i32 rows = 0;
            i32 cols=0;

            AlignedObjectArray<double> allValues;
            if (slot>=0)
            {
                tuk lineChar;
                while (lineChar = fileIO.readLine(slot, lineBuffer, MYLINELENGTH))
                {
                    rows=0;
                    tuk* values = urdfStrSplit(lineChar, ",");
                    if (values)
                    {
                        i32 index = 0;
                        tuk value;
                        while (value = values[index++])
                        {
                            STxt strval(value);
                            double v;
                            if(sscanf(value, "%lf", &v) == 1)
                            {
                                //printf("strlen = %d\n", strval.length());
                                //printf("value[%d,%d]=%s or (%f)", cols,rows,value, v);
                                allValues.push_back(v);
                                rows++;
                            }
                        }
                    }
                    cols++;

                }
                printf("done, rows=%d, cols=%d\n", rows, cols);
                i32 width = rows-1;
                s_gridSize = rows;
                s_gridSpacing = 0.2;
                s_gridHeightScale = 0.2;
                fileIO.fileClose(slot);
                long nElements = ((long)s_gridSize) * s_gridSize;
                //	std::cerr << "  nElements = " << nElements << "\n";

                i32 bytesPerElement = getByteSize(type);
                //	std::cerr << "  bytesPerElement = " << bytesPerElement << "\n";
                Assert(bytesPerElement > 0 && "bad bytes per element");

                long nBytes = nElements * bytesPerElement;
                //	std::cerr << "  nBytes = " << nBytes << "\n";
                byte_t * raw = new byte_t[nBytes];
                Assert(raw && "out of memory");

                byte_t * p = raw;
                for (i32 i = 0; i < width; ++i)
                {
                    float x = i * s_gridSpacing;
                    for (i32 j = 0; j < width; ++j)
                    {
                        float y = j * s_gridSpacing;
                        float z = allValues[i+width*j];
                        convertFromFloat(p, z, type);
						// update min/max
						if (!i && !j) {
							minHeight = z;
							maxHeight = z;
						}
						else {
							if (z < minHeight) {
								minHeight = z;
							}
							if (z > maxHeight) {
								maxHeight = z;
							}
						}
                        p += bytesPerElement;
                    }
                }
                return raw;
            }
            printf("found=%d",found);
        }
    } else
    {
        if (model==eRadial)
        {
            s_gridSize = 16 + 1;  // must be (2^N) + 1
            s_gridSpacing = 0.5;
            s_gridHeightScale = 0.02;
        } else
        {
            s_gridSize = 256 + 1;  // must be (2^N) + 1
            s_gridSpacing = 0.5;
            s_gridHeightScale = 0.02;
        }
        //	std::cerr << "\nRegenerating terrain\n";
        //	std::cerr << "  model = " << model << "\n";
        //	std::cerr << "  type = " << type << "\n";

        long nElements = ((long)s_gridSize) * s_gridSize;
        //	std::cerr << "  nElements = " << nElements << "\n";

        i32 bytesPerElement = getByteSize(type);
        //	std::cerr << "  bytesPerElement = " << bytesPerElement << "\n";
        Assert(bytesPerElement > 0 && "bad bytes per element");

        long nBytes = nElements * bytesPerElement;
        //	std::cerr << "  nBytes = " << nBytes << "\n";
        byte_t * raw = new byte_t[nBytes];
        Assert(raw && "out of memory");

        // reseed randomization every 30 seconds
        //	srand(time(NULL) / 30);

        // populate based on model
        switch (model) {
        case eRadial:
            setRadial(raw, bytesPerElement, type);
            break;

        case eFractal:
            for (i32 i = 0; i < nBytes; i++)
            {
                raw[i] = 0;
            }
            setFractal(raw, bytesPerElement, type, s_gridSize - 1);
            break;

        default:
            Assert(!"bad model type");
        }

        //		std::cerr << "final grid:\n";
        //dumpGrid(raw, bytesPerElement, type, s_gridSize - 1);

        // find min/max
        for (i32 i = 0; i < s_gridSize; ++i) {
            for (i32 j = 0; j < s_gridSize; ++j) {
                Scalar z = getGridHeight(raw, i, j, type);
                //			std::cerr << "i=" << i << ", j=" << j << ": z=" << z << "\n";

                // update min/max
                if (!i && !j) {
                    minHeight = z;
                    maxHeight = z;
                }
                else {
                    if (z < minHeight) {
                        minHeight = z;
                    }
                    if (z > maxHeight) {
                        maxHeight = z;
                    }
                }
            }
        }

        if (maxHeight < -minHeight) {
            maxHeight = -minHeight;
        }
        if (minHeight > -maxHeight) {
            minHeight = -maxHeight;
        }

        //	std::cerr << "  minHeight = " << minHeight << "\n";
        //	std::cerr << "  maxHeight = " << maxHeight << "\n";
        return raw;
    }
	return 0;
}



////////////////////////////////////////////////////////////////////////////////
//
//	TerrainDemo class
//
////////////////////////////////////////////////////////////////////////////////

/// class that demonstrates the HeightfieldTerrainShape object
class HeightfieldExample : public CommonRigidBodyMTBase//CommonRigidBodyBase
{
public:
	// constructor, destructor ---------------------------------------------
	HeightfieldExample(struct GUIHelperInterface* helper);
	virtual ~HeightfieldExample();

	virtual void initPhysics();

	// public class methods ------------------------------------------------

	void castRays();

	void stepSimulation(float deltaTime);

	void resetCamera()
	{
		float dist = 15;
		float pitch = -32;
		float yaw = 35;
		float targetPos[3] = { 0, 0, 0 };
		m_guiHelper->resetCamera(dist, yaw, pitch, targetPos[0], targetPos[1], targetPos[2]);
	}

private:
	// private helper methods ----------------------------------------------
	void resetPhysics(void);
	void clearWorld(void);

	// private data members ------------------------------------------------
	i32					m_upAxis;
	PHY_ScalarType				m_type;
	eTerrainModel				m_model;
	byte_t *				m_rawHeightfieldData;
	Scalar				m_minHeight;
	Scalar				m_maxHeight;
	float					m_phase;	// for dynamics
	bool					m_isDynamic;
	HeightfieldTerrainShape * m_heightfieldShape;
};


#define HEIGHTFIELD_TYPE_COUNT 4
eTerrainModel gHeightfieldType = eRadial;

void setHeightfieldTypeComboBoxCallback(i32 combobox, tukk item, uk userPointer)
{
	tukk* items = static_cast<tukk*>(userPointer);
	for (i32 i = 0; i < HEIGHTFIELD_TYPE_COUNT; ++i)
	{
		if (strcmp(item, items[i]) == 0)
		{
			gHeightfieldType = static_cast<eTerrainModel>(i);
			break;
		}
	}
}



HeightfieldExample::HeightfieldExample(struct GUIHelperInterface* helper)
	: CommonRigidBodyMTBase(helper),
	m_upAxis(1),
	m_type(PHY_FLOAT),
	m_model(eFractal),
	m_rawHeightfieldData(nullptr),
	m_phase(0.0),
	m_isDynamic(true),
	m_heightfieldShape(0)
{
	{
		// create a combo box for selecting the solver type
		static tukk sHeightfieldTypeComboBoxItems[HEIGHTFIELD_TYPE_COUNT];
		for (i32 i = 0; i < HEIGHTFIELD_TYPE_COUNT; ++i)
		{
			eTerrainModel heightfieldType = static_cast<eTerrainModel>(i);
			sHeightfieldTypeComboBoxItems[i] = getTerrainTypeName(heightfieldType);
		}
		ComboBoxParams comboParams;
		comboParams.m_userPointer = sHeightfieldTypeComboBoxItems;
		comboParams.m_numItems = HEIGHTFIELD_TYPE_COUNT;
		comboParams.m_startItem = gHeightfieldType;
		comboParams.m_items = sHeightfieldTypeComboBoxItems;
		comboParams.m_callback = setHeightfieldTypeComboBoxCallback;
		m_guiHelper->getParameterInterface()->registerComboBox(comboParams);
	}
}



HeightfieldExample::~HeightfieldExample(void)
{
	clearWorld();


}


class MyTriangleCollector3 : public TriangleCallback
{
public:
	AlignedObjectArray<GLInstanceVertex>* m_pVerticesOut;
	AlignedObjectArray<i32>* m_pIndicesOut;

	MyTriangleCollector3()
	{
		m_pVerticesOut = 0;
		m_pIndicesOut = 0;
	}

	virtual void processTriangle(Vec3* tris, i32 partId, i32 triangleIndex)
	{
		for (i32 k = 0; k < 3; k++)
		{
			GLInstanceVertex v;
			v.xyzw[3] = 0;
			v.uv[0] = v.uv[1] = 0.5f;
			Vec3 normal = (tris[0] - tris[1]).cross(tris[0] - tris[2]);
			normal.safeNormalize();
			for (i32 l = 0; l < 3; l++)
			{
				v.xyzw[l] = tris[k][l];
				v.normal[l] = normal[l];
			}
			m_pIndicesOut->push_back(m_pVerticesOut->size());
			m_pVerticesOut->push_back(v);
		}
	}
};


#define NUMRAYS2 500
#define USE_PARALLEL_RAYCASTS 1

class RaycastBar3
{
public:
	Vec3 source[NUMRAYS2];
	Vec3 dest[NUMRAYS2];
	Vec3 direction[NUMRAYS2];
	Vec3 hit[NUMRAYS2];
	Vec3 normal[NUMRAYS2];
	struct GUIHelperInterface* m_guiHelper;

	i32 frame_counter;
	i32 ms;
	i32 sum_ms;
	i32 sum_ms_samples;
	i32 min_ms;
	i32 max_ms;

#ifdef USE_DRX3D_CLOCK
	Clock frame_timer;
#endif  //USE_DRX3D_CLOCK

	Scalar dx;
	Scalar min_x;
	Scalar max_x;
	Scalar max_y;
	Scalar sign;

	RaycastBar3()
	{
		m_guiHelper = 0;
		ms = 0;
		max_ms = 0;
		min_ms = 9999;
		sum_ms_samples = 0;
		sum_ms = 0;
	}

	RaycastBar3(Scalar ray_length, Scalar z, Scalar max_y, struct GUIHelperInterface* guiHelper, i32 upAxisIndex)
	{

		m_guiHelper = guiHelper;
		frame_counter = 0;
		ms = 0;
		max_ms = 0;
		min_ms = 9999;
		sum_ms_samples = 0;
		sum_ms = 0;
		dx = 10.0;
		min_x = 0;
		max_x = 0;
		this->max_y = max_y;
		sign = 1.0;
		Scalar dalpha = 2 * SIMD_2_PI / NUMRAYS2;
		for (i32 i = 0; i < NUMRAYS2; i++)
		{
			Scalar alpha = dalpha * i;
			// rotate around by alpha degrees y
			Vec3 upAxis(0, 0, 0);
			upAxis[upAxisIndex] = 1;

			Quat q(upAxis, alpha);
			direction[i] = Vec3(1.0, 0.0, 0.0);
			direction[i] = quatRotate(q, direction[i]);
			direction[i] = direction[i] * ray_length;

			if (upAxisIndex == 1)
			{
				source[i] = Vec3(min_x, max_y, z);
			}
			else
			{
				source[i] = Vec3(min_x, z, max_y);
			}
			dest[i] = source[i] + direction[i];
			dest[i][upAxisIndex] = -1000;
			normal[i] = Vec3(1.0, 0.0, 0.0);
		}
	}

	void move(Scalar dt)
	{
		if (dt > Scalar(1.0 / 60.0))
			dt = Scalar(1.0 / 60.0);
		for (i32 i = 0; i < NUMRAYS2; i++)
		{
			source[i][0] += dx * dt * sign;
			dest[i][0] += dx * dt * sign;
		}
		if (source[0][0] < min_x)
			sign = 1.0;
		else if (source[0][0] > max_x)
			sign = -1.0;
	}

	void castRays(CollisionWorld* cw, i32 iBegin, i32 iEnd)
	{
		if (m_guiHelper==0)
			return;

		for (i32 i = iBegin; i < iEnd; ++i)
		{
			CollisionWorld::ClosestRayResultCallback cb(source[i], dest[i]);

			{
				DRX3D_PROFILE("cw->rayTest");
				//to disable raycast accelerator, uncomment next line
				//cb.m_flags |= TriangleRaycastCallback::kF_DisableHeightfieldAccelerator;
				cw->rayTest(source[i], dest[i], cb);
			}
			if (cb.hasHit())
			{
				hit[i] = cb.m_hitPointWorld;
				normal[i] = cb.m_hitNormalWorld;
				normal[i].normalize();
			}
			else
			{
				hit[i] = dest[i];
				normal[i] = Vec3(1.0, 0.0, 0.0);
			}
		}
	}

	struct CastRaysLoopBody : public IParallelForBody
	{
		CollisionWorld* mWorld;
		RaycastBar3* mRaycasts;

		CastRaysLoopBody(CollisionWorld* cw, RaycastBar3* rb) : mWorld(cw), mRaycasts(rb) {}

		void forLoop(i32 iBegin, i32 iEnd) const
		{
			mRaycasts->castRays(mWorld, iBegin, iEnd);
		}
	};

	void cast(CollisionWorld* cw, bool multiThreading = false)
	{
		DRX3D_PROFILE("cast");

#ifdef USE_DRX3D_CLOCK
		frame_timer.reset();
#endif  //USE_DRX3D_CLOCK

#ifdef BATCH_RAYCASTER
		if (!gBatchRaycaster)
			return;

		gBatchRaycaster->clearRays();
		for (i32 i = 0; i < NUMRAYS; i++)
		{
			gBatchRaycaster->addRay(source[i], dest[i]);
		}
		gBatchRaycaster->performBatchRaycast();
		for (i32 i = 0; i < gBatchRaycaster->getNumRays(); i++)
		{
			const SpuRaycastTaskWorkUnitOut& out = (*gBatchRaycaster)[i];
			hit[i].setInterpolate3(source[i], dest[i], out.hitFraction);
			normal[i] = out.hitNormal;
			normal[i].normalize();
		}
#else
#if USE_PARALLEL_RAYCASTS
		if (multiThreading)
		{
			CastRaysLoopBody rayLooper(cw, this);
			i32 grainSize = 20;  // number of raycasts per task
			ParallelFor(0, NUMRAYS2, grainSize, rayLooper);
		}
		else
#endif  // USE_PARALLEL_RAYCASTS
		{
			// single threaded
			castRays(cw, 0, NUMRAYS2);
		}
#ifdef USE_DRX3D_CLOCK
		ms += frame_timer.getTimeMilliseconds();
#endif  //USE_DRX3D_CLOCK
		frame_counter++;
		if (frame_counter > 50)
		{
			min_ms = ms < min_ms ? ms : min_ms;
			max_ms = ms > max_ms ? ms : max_ms;
			sum_ms += ms;
			sum_ms_samples++;
			Scalar mean_ms = (Scalar)sum_ms / (Scalar)sum_ms_samples;
			printf("%d rays in %d ms %d %d %f\n", NUMRAYS2 * frame_counter, ms, min_ms, max_ms, mean_ms);
			ms = 0;
			frame_counter = 0;
		}
#endif
	}

	void draw()
	{
		if (m_guiHelper)
		{
			AlignedObjectArray<u32> indices;
			AlignedObjectArray<Vec3FloatData> points;

			float lineColor[4] = { 1, 0.4, .4, 1 };

			for (i32 i = 0; i < NUMRAYS2; i++)
			{
				Vec3FloatData s, h;
				for (i32 w = 0; w < 4; w++)
				{
					s.m_floats[w] = source[i][w];
					h.m_floats[w] = hit[i][w];
				}

				points.push_back(s);
				points.push_back(h);
				indices.push_back(indices.size());
				indices.push_back(indices.size());
			}

			m_guiHelper->getRenderInterface()->drawLines(&points[0].m_floats[0], lineColor, points.size(), sizeof(Vec3FloatData), &indices[0], indices.size(), 1);
		}

	}
};

static RaycastBar3 raycastBar;

void HeightfieldExample::castRays()
{
#ifdef DRX3D_THREADSAFE
	raycastBar.cast(m_dynamicsWorld, true);
#else
	raycastBar.cast(m_dynamicsWorld, false);
#endif
}

void HeightfieldExample::stepSimulation(float deltaTime)
{
	castRays();

	raycastBar.draw();

	// if dynamic and radial, go ahead and update the field
	if (m_rawHeightfieldData && m_isDynamic && eRadial == m_model && m_heightfieldShape)
	{
		AlignedObjectArray<GLInstanceVertex> gfxVertices;
		AlignedObjectArray<i32> indices;
		i32 strideInBytes = 9 * sizeof(float);

		m_phase += s_deltaPhase * deltaTime;
		if (m_phase > 2.0 * SIMD_PI) {
			m_phase -= 2.0 * SIMD_PI;
		}
		i32 bpe = getByteSize(m_type);
		Assert(bpe > 0 && "Bad bytes per element");
		setRadial(m_rawHeightfieldData, bpe, m_type, m_phase);

		MyTriangleCollector3  col;
		col.m_pVerticesOut = &gfxVertices;
		col.m_pIndicesOut = &indices;
		Vec3 aabbMin, aabbMax;
		for (i32 k = 0; k < 3; k++)
		{
			aabbMin[k] = -DRX3D_LARGE_FLOAT;
			aabbMax[k] = DRX3D_LARGE_FLOAT;
		}
		m_heightfieldShape->processAllTriangles(&col, aabbMin, aabbMax);
		if (gfxVertices.size() && indices.size())
		{
			m_guiHelper->getRenderInterface()->updateShape(m_heightfieldShape->getUserIndex(), &gfxVertices[0].xyzw[0], gfxVertices.size());
		}
	}

	if (m_model != gHeightfieldType)
	{
		m_model = gHeightfieldType;
		resetPhysics();
	}
	CommonRigidBodyMTBase::stepSimulation(deltaTime);
}

////////////////////////////////////////////////////////////////////////////////
//
//	TerrainDemo -- public class methods
//
////////////////////////////////////////////////////////////////////////////////

/// one-time class and physics initialization
void HeightfieldExample::initPhysics()
{
	//	std::cerr << "initializing...\n";

	createEmptyDynamicsWorld();
	m_guiHelper->createPhysicsDebugDrawer(m_dynamicsWorld);
	m_upAxis = 2;		// start with Y-axis as "up"
	m_guiHelper->setUpAxis(m_upAxis);

	raycastBar = RaycastBar3(2500.0, 0, 2.0, m_guiHelper, m_upAxis);
	// set up basic state


	m_type = PHY_FLOAT;
	m_model = gHeightfieldType;
	m_isDynamic = true;

	// set up the physics world

	// initialize axis- or type-dependent physics from here
	this->resetPhysics();

}



////////////////////////////////////////////////////////////////////////////////
//
//	TerrainDemo -- private helper methods
//
////////////////////////////////////////////////////////////////////////////////

/// called whenever key terrain attribute is changed
void HeightfieldExample::resetPhysics(void)
{
	m_guiHelper->removeAllGraphicsInstances();

	// remove old heightfield
	clearWorld();

	// reset gravity to point in appropriate direction
	m_dynamicsWorld->setGravity(getUpVector(m_upAxis, 0.0, -s_gravity));

	// get new heightfield of appropriate type
	m_rawHeightfieldData =
		getRawHeightfieldData(m_model, m_type, m_minHeight, m_maxHeight);
	Assert(m_rawHeightfieldData && "failed to create raw heightfield");

	bool flipQuadEdges = false;
	m_heightfieldShape =
		new HeightfieldTerrainShape(s_gridSize, s_gridSize,
			m_rawHeightfieldData,
			s_gridHeightScale,
			m_minHeight, m_maxHeight,
			m_upAxis, m_type, flipQuadEdges);
	Assert(m_heightfieldShape && "null heightfield");

	// set origin to middle of heightfield
	Transform2 tr;
	tr.setIdentity();
	tr.setOrigin(Vec3(0, 0, -4));

	if (m_model== eImageFile)
	{
		
		DefaultFileIO fileIO;
		char relativeFileName[1024];
		i32 found = fileIO.findFile("heightmaps/gimp_overlay_out.png", relativeFileName, 1024);

		b3AlignedObjectArray<char> buffer;
		buffer.reserve(1024);
		i32 fileId = fileIO.fileOpen(relativeFileName, "rb");
		if (fileId >= 0)
		{
			i32 size = fileIO.getFileSize(fileId);
			if (size>0)
			{
				buffer.resize(size);
				i32 actual = fileIO.fileRead(fileId, &buffer[0], size);
				if (actual != size)
				{
					drx3DWarning("Насовпадение размера файла STL!\n");
					buffer.resize(0);
				}
			}
			fileIO.fileClose(fileId);
		}

		if (buffer.size())
		{
			i32 width, height, n;


			u8* image = stbi_load_from_memory((u8k*)&buffer[0], buffer.size(), &width, &height, &n, 3);
			if (image)
			{
				i32 texId = m_guiHelper->registerTexture(image, width, height);
				m_heightfieldShape->setUserIndex2(texId);
			}
		}
	}
	if (m_upAxis == 2)
		m_heightfieldShape->setFlipTriangleWinding(true);
	//buildAccelerator is optional, it may not support all features.
	m_heightfieldShape->buildAccelerator();

	// scale the shape
	Vec3 localScaling = getUpVector(m_upAxis, s_gridSpacing, 1.0);
	m_heightfieldShape->setLocalScaling(localScaling);

	// stash this shape away
	m_collisionShapes.push_back(m_heightfieldShape);



	// create ground object
	float mass = 0.0;
	RigidBody* body = createRigidBody(mass, tr, m_heightfieldShape);
	double color[4]={1,1,1,1};

	m_guiHelper->autogenerateGraphicsObjects(m_dynamicsWorld);
	m_guiHelper->changeRGBAColor(body->getUserIndex(),color);
}


/// removes all objects and shapes from the world
void HeightfieldExample::clearWorld(void)
{
	if (m_dynamicsWorld)
	{
		//remove the rigidbodies from the dynamics world and delete them
		i32 i;
		for (i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
		{
			CollisionObject2* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
			RigidBody* body = RigidBody::upcast(obj);
			if (body && body->getMotionState())
			{
				delete body->getMotionState();
			}
			m_dynamicsWorld->removeCollisionObject(obj);
			delete obj;
		}

		//delete collision shapes
		for (i32 j = 0; j < m_collisionShapes.size(); j++)
		{
			CollisionShape* shape = m_collisionShapes[j];
			delete shape;
		}
		m_collisionShapes.clear();

		// delete raw heightfield data
		delete[] m_rawHeightfieldData;
		m_rawHeightfieldData = nullptr;
	}
}

CommonExampleInterface* HeightfieldExampleCreateFunc(CommonExampleOptions& options)
{
	return new HeightfieldExample(options.m_guiHelper);
}

D3_STANDALONE_EXAMPLE(HeightfieldExampleCreateFunc)

