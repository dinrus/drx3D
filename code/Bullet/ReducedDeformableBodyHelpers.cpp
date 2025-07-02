#include <drx3D/Physics/SoftBody/RDB/ReducedDeformableBodyHelpers.h>
#include  <drx3D/Physics/SoftBody/SoftBodyHelpers.h>
#include <iostream>
#include <string>
#include <sstream>

ReducedDeformableBody* ReducedDeformableBodyHelpers::createReducedDeformableObject(SoftBodyWorldInfo& worldInfo, const STxt& file_path, const STxt& vtk_file, i32k num_modes, bool rigid_only) {
	STxt filename = file_path + vtk_file;
	ReducedDeformableBody* rsb = ReducedDeformableBodyHelpers::createFromVtkFile(worldInfo, filename.c_str());
	
	rsb->setReducedModes(num_modes, rsb->m_nodes.size());
	ReducedDeformableBodyHelpers::readReducedDeformableInfoFromFiles(rsb, file_path.c_str());
	
	rsb->disableReducedModes(rigid_only);

	return rsb;
}

ReducedDeformableBody* ReducedDeformableBodyHelpers::createFromVtkFile(SoftBodyWorldInfo& worldInfo, tukk vtk_file)
{
	std::ifstream fs;
	fs.open(vtk_file);
	Assert(fs);

	typedef AlignedObjectArray<i32> Index;
	STxt line;
	AlignedObjectArray<Vec3> X;
	Vec3 position;
	AlignedObjectArray<Index> indices;
	bool reading_points = false;
	bool reading_tets = false;
	size_t n_points = 0;
	size_t n_tets = 0;
	size_t x_count = 0;
	size_t indices_count = 0;
	while (std::getline(fs, line))
	{
		std::stringstream ss(line);
		if (line.size() == (size_t)(0))
		{
		}
		else if (line.substr(0, 6) == "POINTS")
		{
			reading_points = true;
			reading_tets = false;
			ss.ignore(128, ' ');  // ignore "POINTS"
			ss >> n_points;
			X.resize(n_points);
		}
		else if (line.substr(0, 5) == "CELLS")
		{
			reading_points = false;
			reading_tets = true;
			ss.ignore(128, ' ');  // ignore "CELLS"
			ss >> n_tets;
			indices.resize(n_tets);
		}
		else if (line.substr(0, 10) == "CELL_TYPES")
		{
			reading_points = false;
			reading_tets = false;
		}
		else if (reading_points)
		{
			Scalar p;
			ss >> p;
			position.setX(p);
			ss >> p;
			position.setY(p);
			ss >> p;
			position.setZ(p);
			//printf("v %f %f %f\n", position.getX(), position.getY(), position.getZ());
			X[x_count++] = position;
		}
		else if (reading_tets)
		{
			i32 d;
			ss >> d;
			if (d != 4)
			{
				printf("Load deformable failed: Only Tetrahedra are supported in VTK file.\n");
				fs.close();
				return 0;
			}
			ss.ignore(128, ' ');  // ignore "4"
			Index tet;
			tet.resize(4);
			for (size_t i = 0; i < 4; i++)
			{
				ss >> tet[i];
				//printf("%d ", tet[i]);
			}
			//printf("\n");
			indices[indices_count++] = tet;
		}
	}
	ReducedDeformableBody* rsb = new ReducedDeformableBody(&worldInfo, n_points, &X[0], 0);

	for (i32 i = 0; i < n_tets; ++i)
	{
		const Index& ni = indices[i];
		rsb->appendTetra(ni[0], ni[1], ni[2], ni[3]);
		{
			rsb->appendLink(ni[0], ni[1], 0, true);
			rsb->appendLink(ni[1], ni[2], 0, true);
			rsb->appendLink(ni[2], ni[0], 0, true);
			rsb->appendLink(ni[0], ni[3], 0, true);
			rsb->appendLink(ni[1], ni[3], 0, true);
			rsb->appendLink(ni[2], ni[3], 0, true);
		}
	}

	SoftBodyHelpers::generateBoundaryFaces(rsb);
	rsb->initializeDmInverse();
	rsb->m_tetraScratches.resize(rsb->m_tetras.size());
	rsb->m_tetraScratchesTn.resize(rsb->m_tetras.size());
	printf("Nodes:  %u\r\n", rsb->m_nodes.size());
	printf("Links:  %u\r\n", rsb->m_links.size());
	printf("Faces:  %u\r\n", rsb->m_faces.size());
	printf("Tetras: %u\r\n", rsb->m_tetras.size());

	fs.close();

	return rsb;
}

void ReducedDeformableBodyHelpers::readReducedDeformableInfoFromFiles(ReducedDeformableBody* rsb, tukk file_path)
{
	// read in eigenmodes, stiffness and mass matrices
	STxt eigenvalues_file = STxt(file_path) + "eigenvalues.bin";
	ReducedDeformableBodyHelpers::readBinaryVec(rsb->m_eigenvalues, rsb->m_nReduced, eigenvalues_file.c_str());

	STxt Kr_file = STxt(file_path) + "K_r_diag_mat.bin";
	ReducedDeformableBodyHelpers::readBinaryVec(rsb->m_Kr,  rsb->m_nReduced, Kr_file.c_str());

	// STxt Mr_file = STxt(file_path) + "M_r_diag_mat.bin";
	// ReducedDeformableBodyHelpers::readBinaryVec(rsb->m_Mr, rsb->m_nReduced, Mr_file.c_str());

	STxt modes_file = STxt(file_path) + "modes.bin";
	ReducedDeformableBodyHelpers::readBinaryMat(rsb->m_modes, rsb->m_nReduced, 3 * rsb->m_nFull, modes_file.c_str());
	
	// read in full nodal mass
	STxt M_file = STxt(file_path) + "M_diag_mat.bin";
	AlignedObjectArray<Scalar> mass_array;
	ReducedDeformableBodyHelpers::readBinaryVec(mass_array, rsb->m_nFull, M_file.c_str());
	rsb->setMassProps(mass_array);
	
	// calculate the inertia tensor in the local frame 
 	rsb->setInertiaProps();

	// other internal initialization
	rsb->internalInitialization();
}

// read in a vector from the binary file
void ReducedDeformableBodyHelpers::readBinaryVec(ReducedDeformableBody::tDenseArray& vec, 
																				  	 u32k n_size, 				// #entries read
																						 tukk file)
{
	std::ifstream f_in(file, std::ios::in | std::ios::binary);
	// first get size
	u32 size=0;
	f_in.read((tuk)&size, 4);//sizeof(u32));
	Assert(size >= n_size); 	// make sure the #requested mode is smaller than the #available modes

	// read data
	vec.resize(n_size);
	double temp;
	for (u32 i = 0; i < n_size; ++i)
	{
		f_in.read((tuk)&temp, sizeof(double));
		vec[i] = Scalar(temp);
	}
  f_in.close();
}

// read in a matrix from the binary file
void ReducedDeformableBodyHelpers::readBinaryMat(ReducedDeformableBody::tDenseMatrix& mat, 
																						 u32k n_modes, 		// #modes, outer array size
																						 u32k n_full, 		// inner array size
																						 tukk file)
{
	std::ifstream f_in(file, std::ios::in | std::ios::binary);
	// first get size
	u32 v_size=0;
	f_in.read((tuk)&v_size, 4);//sizeof(u32));
	Assert(v_size >= n_modes * n_full); 	// make sure the #requested mode is smaller than the #available modes

	// read data
	mat.resize(n_modes);
	for (i32 i = 0; i < n_modes; ++i) 
	{
		for (i32 j = 0; j < n_full; ++j)
		{
			double temp;
			f_in.read((tuk)&temp, sizeof(double));

			if (mat[i].size() != n_modes)
				mat[i].resize(n_full);
			mat[i][j] = Scalar(temp);
		}
	}
  f_in.close();
}

void ReducedDeformableBodyHelpers::calculateLocalInertia(Vec3& inertia, const Scalar mass, const Vec3& half_extents, const Vec3& margin)
{
	Scalar lx = Scalar(2.) * (half_extents[0] + margin[0]);
	Scalar ly = Scalar(2.) * (half_extents[1] + margin[1]);
	Scalar lz = Scalar(2.) * (half_extents[2] + margin[2]);

	inertia.setVal(mass / (Scalar(12.0)) * (ly * ly + lz * lz),
								   mass / (Scalar(12.0)) * (lx * lx + lz * lz),
								   mass / (Scalar(12.0)) * (lx * lx + ly * ly));
}
