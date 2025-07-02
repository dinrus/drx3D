
#include <drx3D/Physics/Collision/Shapes/ShapeHull.h>
#include <drx3D/Maths/Linear/ConvexHull.h>

#define NUM_UNITSPHERE_POINTS 42
#define NUM_UNITSPHERE_POINTS_HIGHRES 256

ShapeHull::ShapeHull(const ConvexShape* shape)
{
	m_shape = shape;
	m_vertices.clear();
	m_indices.clear();
	m_numIndices = 0;
}

ShapeHull::~ShapeHull()
{
	m_indices.clear();
	m_vertices.clear();
}

bool ShapeHull::buildHull(Scalar /*margin*/, i32 highres)
{
	
	i32 numSampleDirections = highres ? NUM_UNITSPHERE_POINTS_HIGHRES : NUM_UNITSPHERE_POINTS;
	Vec3 supportPoints[NUM_UNITSPHERE_POINTS_HIGHRES + MAX_PREFERRED_PENETRATION_DIRECTIONS * 2];
	i32 i;
	for (i = 0; i < numSampleDirections; i++)
	{
		supportPoints[i] = m_shape->localGetSupportingVertex(getUnitSpherePoints(highres)[i]);
	}

	i32 numPDA = m_shape->getNumPreferredPenetrationDirections();
	if (numPDA)
	{
		for (i32 s = 0; s < numPDA; s++)
		{
			Vec3 norm;
			m_shape->getPreferredPenetrationDirection(s, norm);
			supportPoints[i++] = m_shape->localGetSupportingVertex(norm);
			numSampleDirections++;
		}
	}
	HullDesc hd;
	hd.mFlags = QF_TRIANGLES;
	hd.mVcount = static_cast<u32>(numSampleDirections);

#ifdef DRX3D_USE_DOUBLE_PRECISION
	hd.mVertices = &supportPoints[0];
	hd.mVertexStride = sizeof(Vec3);
#else
	hd.mVertices = &supportPoints[0];
	hd.mVertexStride = sizeof(Vec3);
#endif

	HullLibrary hl;
	HullResult hr;
	if (hl.CreateConvexHull(hd, hr) == QE_FAIL)
	{
		return false;
	}

	m_vertices.resize(static_cast<i32>(hr.mNumOutputVertices));

	for (i = 0; i < static_cast<i32>(hr.mNumOutputVertices); i++)
	{
		m_vertices[i] = hr.m_OutputVertices[i];
	}
	m_numIndices = hr.mNumIndices;
	m_indices.resize(static_cast<i32>(m_numIndices));
	for (i = 0; i < static_cast<i32>(m_numIndices); i++)
	{
		m_indices[i] = hr.m_Indices[i];
	}

	// free temporary hull result that we just copied
	hl.ReleaseResult(hr);

	return true;
}

i32 ShapeHull::numTriangles() const
{
	return static_cast<i32>(m_numIndices / 3);
}

i32 ShapeHull::numVertices() const
{
	return m_vertices.size();
}

i32 ShapeHull::numIndices() const
{
	return static_cast<i32>(m_numIndices);
}

Vec3* ShapeHull::getUnitSpherePoints(i32 highres)
{
	static Vec3 sUnitSpherePointsHighres[NUM_UNITSPHERE_POINTS_HIGHRES + MAX_PREFERRED_PENETRATION_DIRECTIONS * 2] =
		{
			Vec3(Scalar(0.997604), Scalar(0.067004), Scalar(0.017144)),
			Vec3(Scalar(0.984139), Scalar(-0.086784), Scalar(-0.154427)),
			Vec3(Scalar(0.971065), Scalar(0.124164), Scalar(-0.203224)),
			Vec3(Scalar(0.955844), Scalar(0.291173), Scalar(-0.037704)),
			Vec3(Scalar(0.957405), Scalar(0.212238), Scalar(0.195157)),
			Vec3(Scalar(0.971650), Scalar(-0.012709), Scalar(0.235561)),
			Vec3(Scalar(0.984920), Scalar(-0.161831), Scalar(0.059695)),
			Vec3(Scalar(0.946673), Scalar(-0.299288), Scalar(-0.117536)),
			Vec3(Scalar(0.922670), Scalar(-0.219186), Scalar(-0.317019)),
			Vec3(Scalar(0.928134), Scalar(-0.007265), Scalar(-0.371867)),
			Vec3(Scalar(0.875642), Scalar(0.198434), Scalar(-0.439988)),
			Vec3(Scalar(0.908035), Scalar(0.325975), Scalar(-0.262562)),
			Vec3(Scalar(0.864519), Scalar(0.488706), Scalar(-0.116755)),
			Vec3(Scalar(0.893009), Scalar(0.428046), Scalar(0.137185)),
			Vec3(Scalar(0.857494), Scalar(0.362137), Scalar(0.364776)),
			Vec3(Scalar(0.900815), Scalar(0.132524), Scalar(0.412987)),
			Vec3(Scalar(0.934964), Scalar(-0.241739), Scalar(0.259179)),
			Vec3(Scalar(0.894570), Scalar(-0.103504), Scalar(0.434263)),
			Vec3(Scalar(0.922085), Scalar(-0.376668), Scalar(0.086241)),
			Vec3(Scalar(0.862177), Scalar(-0.499154), Scalar(-0.085330)),
			Vec3(Scalar(0.861982), Scalar(-0.420218), Scalar(-0.282861)),
			Vec3(Scalar(0.818076), Scalar(-0.328256), Scalar(-0.471804)),
			Vec3(Scalar(0.762657), Scalar(-0.179329), Scalar(-0.621124)),
			Vec3(Scalar(0.826857), Scalar(0.019760), Scalar(-0.561786)),
			Vec3(Scalar(0.731434), Scalar(0.206599), Scalar(-0.649817)),
			Vec3(Scalar(0.769486), Scalar(0.379052), Scalar(-0.513770)),
			Vec3(Scalar(0.796806), Scalar(0.507176), Scalar(-0.328145)),
			Vec3(Scalar(0.679722), Scalar(0.684101), Scalar(-0.264123)),
			Vec3(Scalar(0.786854), Scalar(0.614886), Scalar(0.050912)),
			Vec3(Scalar(0.769486), Scalar(0.571141), Scalar(0.285139)),
			Vec3(Scalar(0.707432), Scalar(0.492789), Scalar(0.506288)),
			Vec3(Scalar(0.774560), Scalar(0.268037), Scalar(0.572652)),
			Vec3(Scalar(0.796220), Scalar(0.031230), Scalar(0.604077)),
			Vec3(Scalar(0.837395), Scalar(-0.320285), Scalar(0.442461)),
			Vec3(Scalar(0.848127), Scalar(-0.450548), Scalar(0.278307)),
			Vec3(Scalar(0.775536), Scalar(-0.206354), Scalar(0.596465)),
			Vec3(Scalar(0.816320), Scalar(-0.567007), Scalar(0.109469)),
			Vec3(Scalar(0.741191), Scalar(-0.668690), Scalar(-0.056832)),
			Vec3(Scalar(0.755632), Scalar(-0.602975), Scalar(-0.254949)),
			Vec3(Scalar(0.720311), Scalar(-0.521318), Scalar(-0.457165)),
			Vec3(Scalar(0.670746), Scalar(-0.386583), Scalar(-0.632835)),
			Vec3(Scalar(0.587031), Scalar(-0.219769), Scalar(-0.778836)),
			Vec3(Scalar(0.676015), Scalar(-0.003182), Scalar(-0.736676)),
			Vec3(Scalar(0.566932), Scalar(0.186963), Scalar(-0.802064)),
			Vec3(Scalar(0.618254), Scalar(0.398105), Scalar(-0.677533)),
			Vec3(Scalar(0.653964), Scalar(0.575224), Scalar(-0.490933)),
			Vec3(Scalar(0.525367), Scalar(0.743205), Scalar(-0.414028)),
			Vec3(Scalar(0.506439), Scalar(0.836528), Scalar(-0.208885)),
			Vec3(Scalar(0.651427), Scalar(0.756426), Scalar(-0.056247)),
			Vec3(Scalar(0.641670), Scalar(0.745149), Scalar(0.180908)),
			Vec3(Scalar(0.602643), Scalar(0.687211), Scalar(0.405180)),
			Vec3(Scalar(0.516586), Scalar(0.596999), Scalar(0.613447)),
			Vec3(Scalar(0.602252), Scalar(0.387801), Scalar(0.697573)),
			Vec3(Scalar(0.646549), Scalar(0.153911), Scalar(0.746956)),
			Vec3(Scalar(0.650842), Scalar(-0.087756), Scalar(0.753983)),
			Vec3(Scalar(0.740411), Scalar(-0.497404), Scalar(0.451830)),
			Vec3(Scalar(0.726946), Scalar(-0.619890), Scalar(0.295093)),
			Vec3(Scalar(0.637768), Scalar(-0.313092), Scalar(0.703624)),
			Vec3(Scalar(0.678942), Scalar(-0.722934), Scalar(0.126645)),
			Vec3(Scalar(0.489072), Scalar(-0.867195), Scalar(-0.092942)),
			Vec3(Scalar(0.622742), Scalar(-0.757541), Scalar(-0.194636)),
			Vec3(Scalar(0.596788), Scalar(-0.693576), Scalar(-0.403098)),
			Vec3(Scalar(0.550150), Scalar(-0.582172), Scalar(-0.598287)),
			Vec3(Scalar(0.474436), Scalar(-0.429745), Scalar(-0.768101)),
			Vec3(Scalar(0.372574), Scalar(-0.246016), Scalar(-0.894583)),
			Vec3(Scalar(0.480095), Scalar(-0.026513), Scalar(-0.876626)),
			Vec3(Scalar(0.352474), Scalar(0.177242), Scalar(-0.918787)),
			Vec3(Scalar(0.441848), Scalar(0.374386), Scalar(-0.814946)),
			Vec3(Scalar(0.492389), Scalar(0.582223), Scalar(-0.646693)),
			Vec3(Scalar(0.343498), Scalar(0.866080), Scalar(-0.362693)),
			Vec3(Scalar(0.362036), Scalar(0.745149), Scalar(-0.559639)),
			Vec3(Scalar(0.334131), Scalar(0.937044), Scalar(-0.099774)),
			Vec3(Scalar(0.486925), Scalar(0.871718), Scalar(0.052473)),
			Vec3(Scalar(0.452776), Scalar(0.845665), Scalar(0.281820)),
			Vec3(Scalar(0.399503), Scalar(0.771785), Scalar(0.494576)),
			Vec3(Scalar(0.296469), Scalar(0.673018), Scalar(0.677469)),
			Vec3(Scalar(0.392088), Scalar(0.479179), Scalar(0.785213)),
			Vec3(Scalar(0.452190), Scalar(0.252094), Scalar(0.855286)),
			Vec3(Scalar(0.478339), Scalar(0.013149), Scalar(0.877928)),
			Vec3(Scalar(0.481656), Scalar(-0.219380), Scalar(0.848259)),
			Vec3(Scalar(0.615327), Scalar(-0.494293), Scalar(0.613837)),
			Vec3(Scalar(0.594642), Scalar(-0.650414), Scalar(0.472325)),
			Vec3(Scalar(0.562249), Scalar(-0.771345), Scalar(0.297631)),
			Vec3(Scalar(0.467411), Scalar(-0.437133), Scalar(0.768231)),
			Vec3(Scalar(0.519513), Scalar(-0.847947), Scalar(0.103808)),
			Vec3(Scalar(0.297640), Scalar(-0.938159), Scalar(-0.176288)),
			Vec3(Scalar(0.446727), Scalar(-0.838615), Scalar(-0.311359)),
			Vec3(Scalar(0.331790), Scalar(-0.942437), Scalar(0.040762)),
			Vec3(Scalar(0.413358), Scalar(-0.748403), Scalar(-0.518259)),
			Vec3(Scalar(0.347596), Scalar(-0.621640), Scalar(-0.701737)),
			Vec3(Scalar(0.249831), Scalar(-0.456186), Scalar(-0.853984)),
			Vec3(Scalar(0.131772), Scalar(-0.262931), Scalar(-0.955678)),
			Vec3(Scalar(0.247099), Scalar(-0.042261), Scalar(-0.967975)),
			Vec3(Scalar(0.113624), Scalar(0.165965), Scalar(-0.979491)),
			Vec3(Scalar(0.217438), Scalar(0.374580), Scalar(-0.901220)),
			Vec3(Scalar(0.307983), Scalar(0.554615), Scalar(-0.772786)),
			Vec3(Scalar(0.166702), Scalar(0.953181), Scalar(-0.252021)),
			Vec3(Scalar(0.172751), Scalar(0.844499), Scalar(-0.506743)),
			Vec3(Scalar(0.177630), Scalar(0.711125), Scalar(-0.679876)),
			Vec3(Scalar(0.120064), Scalar(0.992260), Scalar(-0.030482)),
			Vec3(Scalar(0.289640), Scalar(0.949098), Scalar(0.122546)),
			Vec3(Scalar(0.239879), Scalar(0.909047), Scalar(0.340377)),
			Vec3(Scalar(0.181142), Scalar(0.821363), Scalar(0.540641)),
			Vec3(Scalar(0.066986), Scalar(0.719097), Scalar(0.691327)),
			Vec3(Scalar(0.156750), Scalar(0.545478), Scalar(0.823079)),
			Vec3(Scalar(0.236172), Scalar(0.342306), Scalar(0.909353)),
			Vec3(Scalar(0.277541), Scalar(0.112693), Scalar(0.953856)),
			Vec3(Scalar(0.295299), Scalar(-0.121974), Scalar(0.947415)),
			Vec3(Scalar(0.287883), Scalar(-0.349254), Scalar(0.891591)),
			Vec3(Scalar(0.437165), Scalar(-0.634666), Scalar(0.636869)),
			Vec3(Scalar(0.407113), Scalar(-0.784954), Scalar(0.466664)),
			Vec3(Scalar(0.375111), Scalar(-0.888193), Scalar(0.264839)),
			Vec3(Scalar(0.275394), Scalar(-0.560591), Scalar(0.780723)),
			Vec3(Scalar(0.122015), Scalar(-0.992209), Scalar(-0.024821)),
			Vec3(Scalar(0.087866), Scalar(-0.966156), Scalar(-0.241676)),
			Vec3(Scalar(0.239489), Scalar(-0.885665), Scalar(-0.397437)),
			Vec3(Scalar(0.167287), Scalar(-0.965184), Scalar(0.200817)),
			Vec3(Scalar(0.201632), Scalar(-0.776789), Scalar(-0.596335)),
			Vec3(Scalar(0.122015), Scalar(-0.637971), Scalar(-0.760098)),
			Vec3(Scalar(0.008054), Scalar(-0.464741), Scalar(-0.885214)),
			Vec3(Scalar(-0.116054), Scalar(-0.271096), Scalar(-0.955482)),
			Vec3(Scalar(-0.000727), Scalar(-0.056065), Scalar(-0.998424)),
			Vec3(Scalar(-0.134007), Scalar(0.152939), Scalar(-0.978905)),
			Vec3(Scalar(-0.025900), Scalar(0.366026), Scalar(-0.930108)),
			Vec3(Scalar(0.081231), Scalar(0.557337), Scalar(-0.826072)),
			Vec3(Scalar(-0.002874), Scalar(0.917213), Scalar(-0.398023)),
			Vec3(Scalar(-0.050683), Scalar(0.981761), Scalar(-0.182534)),
			Vec3(Scalar(-0.040536), Scalar(0.710153), Scalar(-0.702713)),
			Vec3(Scalar(-0.139081), Scalar(0.827973), Scalar(-0.543048)),
			Vec3(Scalar(-0.101029), Scalar(0.994010), Scalar(0.041152)),
			Vec3(Scalar(0.069328), Scalar(0.978067), Scalar(0.196133)),
			Vec3(Scalar(0.023860), Scalar(0.911380), Scalar(0.410645)),
			Vec3(Scalar(-0.153521), Scalar(0.736789), Scalar(0.658145)),
			Vec3(Scalar(-0.070002), Scalar(0.591750), Scalar(0.802780)),
			Vec3(Scalar(0.002590), Scalar(0.312948), Scalar(0.949562)),
			Vec3(Scalar(0.090988), Scalar(-0.020680), Scalar(0.995627)),
			Vec3(Scalar(0.088842), Scalar(-0.250099), Scalar(0.964006)),
			Vec3(Scalar(0.083378), Scalar(-0.470185), Scalar(0.878318)),
			Vec3(Scalar(0.240074), Scalar(-0.749764), Scalar(0.616374)),
			Vec3(Scalar(0.210803), Scalar(-0.885860), Scalar(0.412987)),
			Vec3(Scalar(0.077524), Scalar(-0.660524), Scalar(0.746565)),
			Vec3(Scalar(-0.096736), Scalar(-0.990070), Scalar(-0.100945)),
			Vec3(Scalar(-0.052634), Scalar(-0.990264), Scalar(0.127426)),
			Vec3(Scalar(-0.106102), Scalar(-0.938354), Scalar(-0.328340)),
			Vec3(Scalar(0.013323), Scalar(-0.863112), Scalar(-0.504596)),
			Vec3(Scalar(-0.002093), Scalar(-0.936993), Scalar(0.349161)),
			Vec3(Scalar(-0.106297), Scalar(-0.636610), Scalar(-0.763612)),
			Vec3(Scalar(-0.229430), Scalar(-0.463769), Scalar(-0.855546)),
			Vec3(Scalar(-0.245236), Scalar(-0.066175), Scalar(-0.966999)),
			Vec3(Scalar(-0.351587), Scalar(-0.270513), Scalar(-0.896145)),
			Vec3(Scalar(-0.370906), Scalar(0.133108), Scalar(-0.918982)),
			Vec3(Scalar(-0.264360), Scalar(0.346000), Scalar(-0.900049)),
			Vec3(Scalar(-0.151375), Scalar(0.543728), Scalar(-0.825291)),
			Vec3(Scalar(-0.218697), Scalar(0.912741), Scalar(-0.344346)),
			Vec3(Scalar(-0.274507), Scalar(0.953764), Scalar(-0.121635)),
			Vec3(Scalar(-0.259677), Scalar(0.692266), Scalar(-0.673044)),
			Vec3(Scalar(-0.350416), Scalar(0.798810), Scalar(-0.488786)),
			Vec3(Scalar(-0.320170), Scalar(0.941127), Scalar(0.108297)),
			Vec3(Scalar(-0.147667), Scalar(0.952792), Scalar(0.265034)),
			Vec3(Scalar(-0.188061), Scalar(0.860636), Scalar(0.472910)),
			Vec3(Scalar(-0.370906), Scalar(0.739900), Scalar(0.560941)),
			Vec3(Scalar(-0.297143), Scalar(0.585334), Scalar(0.754178)),
			Vec3(Scalar(-0.189622), Scalar(0.428241), Scalar(0.883393)),
			Vec3(Scalar(-0.091272), Scalar(0.098695), Scalar(0.990747)),
			Vec3(Scalar(-0.256945), Scalar(0.228375), Scalar(0.938827)),
			Vec3(Scalar(-0.111761), Scalar(-0.133251), Scalar(0.984696)),
			Vec3(Scalar(-0.118006), Scalar(-0.356253), Scalar(0.926725)),
			Vec3(Scalar(-0.119372), Scalar(-0.563896), Scalar(0.817029)),
			Vec3(Scalar(0.041228), Scalar(-0.833949), Scalar(0.550010)),
			Vec3(Scalar(-0.121909), Scalar(-0.736543), Scalar(0.665172)),
			Vec3(Scalar(-0.307681), Scalar(-0.931160), Scalar(-0.195026)),
			Vec3(Scalar(-0.283679), Scalar(-0.957990), Scalar(0.041348)),
			Vec3(Scalar(-0.227284), Scalar(-0.935243), Scalar(0.270890)),
			Vec3(Scalar(-0.293436), Scalar(-0.858252), Scalar(-0.420860)),
			Vec3(Scalar(-0.175767), Scalar(-0.780677), Scalar(-0.599262)),
			Vec3(Scalar(-0.170108), Scalar(-0.858835), Scalar(0.482865)),
			Vec3(Scalar(-0.332854), Scalar(-0.635055), Scalar(-0.696857)),
			Vec3(Scalar(-0.447791), Scalar(-0.445299), Scalar(-0.775128)),
			Vec3(Scalar(-0.470622), Scalar(-0.074146), Scalar(-0.879164)),
			Vec3(Scalar(-0.639417), Scalar(-0.340505), Scalar(-0.689049)),
			Vec3(Scalar(-0.598438), Scalar(0.104722), Scalar(-0.794256)),
			Vec3(Scalar(-0.488575), Scalar(0.307699), Scalar(-0.816313)),
			Vec3(Scalar(-0.379882), Scalar(0.513592), Scalar(-0.769077)),
			Vec3(Scalar(-0.425740), Scalar(0.862775), Scalar(-0.272516)),
			Vec3(Scalar(-0.480769), Scalar(0.875412), Scalar(-0.048439)),
			Vec3(Scalar(-0.467890), Scalar(0.648716), Scalar(-0.600043)),
			Vec3(Scalar(-0.543799), Scalar(0.730956), Scalar(-0.411881)),
			Vec3(Scalar(-0.516284), Scalar(0.838277), Scalar(0.174076)),
			Vec3(Scalar(-0.353343), Scalar(0.876384), Scalar(0.326519)),
			Vec3(Scalar(-0.572875), Scalar(0.614497), Scalar(0.542007)),
			Vec3(Scalar(-0.503600), Scalar(0.497261), Scalar(0.706161)),
			Vec3(Scalar(-0.530920), Scalar(0.754870), Scalar(0.384685)),
			Vec3(Scalar(-0.395884), Scalar(0.366414), Scalar(0.841818)),
			Vec3(Scalar(-0.300656), Scalar(0.001678), Scalar(0.953661)),
			Vec3(Scalar(-0.461060), Scalar(0.146912), Scalar(0.875000)),
			Vec3(Scalar(-0.315486), Scalar(-0.232212), Scalar(0.919893)),
			Vec3(Scalar(-0.323682), Scalar(-0.449187), Scalar(0.832644)),
			Vec3(Scalar(-0.318999), Scalar(-0.639527), Scalar(0.699134)),
			Vec3(Scalar(-0.496771), Scalar(-0.866029), Scalar(-0.055271)),
			Vec3(Scalar(-0.496771), Scalar(-0.816257), Scalar(-0.294377)),
			Vec3(Scalar(-0.456377), Scalar(-0.869528), Scalar(0.188130)),
			Vec3(Scalar(-0.380858), Scalar(-0.827144), Scalar(0.412792)),
			Vec3(Scalar(-0.449352), Scalar(-0.727405), Scalar(-0.518259)),
			Vec3(Scalar(-0.570533), Scalar(-0.551064), Scalar(-0.608632)),
			Vec3(Scalar(-0.656394), Scalar(-0.118280), Scalar(-0.744874)),
			Vec3(Scalar(-0.756696), Scalar(-0.438105), Scalar(-0.484882)),
			Vec3(Scalar(-0.801773), Scalar(-0.204798), Scalar(-0.561005)),
			Vec3(Scalar(-0.785186), Scalar(0.038618), Scalar(-0.617805)),
			Vec3(Scalar(-0.709082), Scalar(0.262399), Scalar(-0.654306)),
			Vec3(Scalar(-0.583412), Scalar(0.462265), Scalar(-0.667383)),
			Vec3(Scalar(-0.616001), Scalar(0.761286), Scalar(-0.201272)),
			Vec3(Scalar(-0.660687), Scalar(0.750204), Scalar(0.020072)),
			Vec3(Scalar(-0.744987), Scalar(0.435823), Scalar(-0.504791)),
			Vec3(Scalar(-0.713765), Scalar(0.605554), Scalar(-0.351373)),
			Vec3(Scalar(-0.686251), Scalar(0.687600), Scalar(0.236927)),
			Vec3(Scalar(-0.680201), Scalar(0.429407), Scalar(0.593732)),
			Vec3(Scalar(-0.733474), Scalar(0.546450), Scalar(0.403814)),
			Vec3(Scalar(-0.591023), Scalar(0.292923), Scalar(0.751445)),
			Vec3(Scalar(-0.500283), Scalar(-0.080757), Scalar(0.861922)),
			Vec3(Scalar(-0.643710), Scalar(0.070115), Scalar(0.761985)),
			Vec3(Scalar(-0.506332), Scalar(-0.308425), Scalar(0.805122)),
			Vec3(Scalar(-0.503015), Scalar(-0.509847), Scalar(0.697573)),
			Vec3(Scalar(-0.482525), Scalar(-0.682105), Scalar(0.549229)),
			Vec3(Scalar(-0.680396), Scalar(-0.716323), Scalar(-0.153451)),
			Vec3(Scalar(-0.658346), Scalar(-0.746264), Scalar(0.097562)),
			Vec3(Scalar(-0.653272), Scalar(-0.646915), Scalar(-0.392948)),
			Vec3(Scalar(-0.590828), Scalar(-0.732655), Scalar(0.337645)),
			Vec3(Scalar(-0.819140), Scalar(-0.518013), Scalar(-0.246166)),
			Vec3(Scalar(-0.900513), Scalar(-0.282178), Scalar(-0.330487)),
			Vec3(Scalar(-0.914953), Scalar(-0.028652), Scalar(-0.402122)),
			Vec3(Scalar(-0.859924), Scalar(0.220209), Scalar(-0.459898)),
			Vec3(Scalar(-0.777185), Scalar(0.613720), Scalar(-0.137836)),
			Vec3(Scalar(-0.805285), Scalar(0.586889), Scalar(0.082728)),
			Vec3(Scalar(-0.872413), Scalar(0.406077), Scalar(-0.271735)),
			Vec3(Scalar(-0.859339), Scalar(0.448072), Scalar(0.246101)),
			Vec3(Scalar(-0.757671), Scalar(0.216320), Scalar(0.615594)),
			Vec3(Scalar(-0.826165), Scalar(0.348139), Scalar(0.442851)),
			Vec3(Scalar(-0.671810), Scalar(-0.162803), Scalar(0.722557)),
			Vec3(Scalar(-0.796504), Scalar(-0.004543), Scalar(0.604468)),
			Vec3(Scalar(-0.676298), Scalar(-0.378223), Scalar(0.631794)),
			Vec3(Scalar(-0.668883), Scalar(-0.558258), Scalar(0.490673)),
			Vec3(Scalar(-0.821287), Scalar(-0.570118), Scalar(0.006994)),
			Vec3(Scalar(-0.767428), Scalar(-0.587810), Scalar(0.255470)),
			Vec3(Scalar(-0.933296), Scalar(-0.349837), Scalar(-0.079865)),
			Vec3(Scalar(-0.982667), Scalar(-0.100393), Scalar(-0.155208)),
			Vec3(Scalar(-0.961396), Scalar(0.160910), Scalar(-0.222938)),
			Vec3(Scalar(-0.934858), Scalar(0.354555), Scalar(-0.006864)),
			Vec3(Scalar(-0.941687), Scalar(0.229736), Scalar(0.245711)),
			Vec3(Scalar(-0.884317), Scalar(0.131552), Scalar(0.447536)),
			Vec3(Scalar(-0.810359), Scalar(-0.219769), Scalar(0.542788)),
			Vec3(Scalar(-0.915929), Scalar(-0.210048), Scalar(0.341743)),
			Vec3(Scalar(-0.816799), Scalar(-0.407192), Scalar(0.408303)),
			Vec3(Scalar(-0.903050), Scalar(-0.392416), Scalar(0.174076)),
			Vec3(Scalar(-0.980325), Scalar(-0.170969), Scalar(0.096586)),
			Vec3(Scalar(-0.995936), Scalar(0.084891), Scalar(0.029441)),
			Vec3(Scalar(-0.960031), Scalar(0.002650), Scalar(0.279283)),
		};
	static Vec3 sUnitSpherePoints[NUM_UNITSPHERE_POINTS + MAX_PREFERRED_PENETRATION_DIRECTIONS * 2] =
		{
			Vec3(Scalar(0.000000), Scalar(-0.000000), Scalar(-1.000000)),
			Vec3(Scalar(0.723608), Scalar(-0.525725), Scalar(-0.447219)),
			Vec3(Scalar(-0.276388), Scalar(-0.850649), Scalar(-0.447219)),
			Vec3(Scalar(-0.894426), Scalar(-0.000000), Scalar(-0.447216)),
			Vec3(Scalar(-0.276388), Scalar(0.850649), Scalar(-0.447220)),
			Vec3(Scalar(0.723608), Scalar(0.525725), Scalar(-0.447219)),
			Vec3(Scalar(0.276388), Scalar(-0.850649), Scalar(0.447220)),
			Vec3(Scalar(-0.723608), Scalar(-0.525725), Scalar(0.447219)),
			Vec3(Scalar(-0.723608), Scalar(0.525725), Scalar(0.447219)),
			Vec3(Scalar(0.276388), Scalar(0.850649), Scalar(0.447219)),
			Vec3(Scalar(0.894426), Scalar(0.000000), Scalar(0.447216)),
			Vec3(Scalar(-0.000000), Scalar(0.000000), Scalar(1.000000)),
			Vec3(Scalar(0.425323), Scalar(-0.309011), Scalar(-0.850654)),
			Vec3(Scalar(-0.162456), Scalar(-0.499995), Scalar(-0.850654)),
			Vec3(Scalar(0.262869), Scalar(-0.809012), Scalar(-0.525738)),
			Vec3(Scalar(0.425323), Scalar(0.309011), Scalar(-0.850654)),
			Vec3(Scalar(0.850648), Scalar(-0.000000), Scalar(-0.525736)),
			Vec3(Scalar(-0.525730), Scalar(-0.000000), Scalar(-0.850652)),
			Vec3(Scalar(-0.688190), Scalar(-0.499997), Scalar(-0.525736)),
			Vec3(Scalar(-0.162456), Scalar(0.499995), Scalar(-0.850654)),
			Vec3(Scalar(-0.688190), Scalar(0.499997), Scalar(-0.525736)),
			Vec3(Scalar(0.262869), Scalar(0.809012), Scalar(-0.525738)),
			Vec3(Scalar(0.951058), Scalar(0.309013), Scalar(0.000000)),
			Vec3(Scalar(0.951058), Scalar(-0.309013), Scalar(0.000000)),
			Vec3(Scalar(0.587786), Scalar(-0.809017), Scalar(0.000000)),
			Vec3(Scalar(0.000000), Scalar(-1.000000), Scalar(0.000000)),
			Vec3(Scalar(-0.587786), Scalar(-0.809017), Scalar(0.000000)),
			Vec3(Scalar(-0.951058), Scalar(-0.309013), Scalar(-0.000000)),
			Vec3(Scalar(-0.951058), Scalar(0.309013), Scalar(-0.000000)),
			Vec3(Scalar(-0.587786), Scalar(0.809017), Scalar(-0.000000)),
			Vec3(Scalar(-0.000000), Scalar(1.000000), Scalar(-0.000000)),
			Vec3(Scalar(0.587786), Scalar(0.809017), Scalar(-0.000000)),
			Vec3(Scalar(0.688190), Scalar(-0.499997), Scalar(0.525736)),
			Vec3(Scalar(-0.262869), Scalar(-0.809012), Scalar(0.525738)),
			Vec3(Scalar(-0.850648), Scalar(0.000000), Scalar(0.525736)),
			Vec3(Scalar(-0.262869), Scalar(0.809012), Scalar(0.525738)),
			Vec3(Scalar(0.688190), Scalar(0.499997), Scalar(0.525736)),
			Vec3(Scalar(0.525730), Scalar(0.000000), Scalar(0.850652)),
			Vec3(Scalar(0.162456), Scalar(-0.499995), Scalar(0.850654)),
			Vec3(Scalar(-0.425323), Scalar(-0.309011), Scalar(0.850654)),
			Vec3(Scalar(-0.425323), Scalar(0.309011), Scalar(0.850654)),
			Vec3(Scalar(0.162456), Scalar(0.499995), Scalar(0.850654))};
	if (highres)
		return sUnitSpherePointsHighres;
	return sUnitSpherePoints;
}
