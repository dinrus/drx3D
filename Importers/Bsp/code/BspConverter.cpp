
#include "../BspConverter.h"
#include "../BspLoader.h"
#include <drx3D/Maths/Linear/Vec3.h>
#include <drx3D/Maths/Linear/GeometryUtil.h>
#include <stdio.h>
#include <string.h>

void BspConverter::convertBsp(BspLoader& bspLoader, float scaling)
{
	{
		float playstartf[3] = {0, 0, 100};

		if (bspLoader.findVectorByName(&playstartf[0], "info_player_start"))
		{
			printf("found playerstart\n");
		}
		else
		{
			if (bspLoader.findVectorByName(&playstartf[0], "info_player_deathmatch"))
			{
				printf("found deatchmatch start\n");
			}
		}

		Vec3 playerStart(playstartf[0], playstartf[1], playstartf[2]);

		playerStart[2] += 20.f;  //start a bit higher

		playerStart *= scaling;

		//progressBegin("Loading bsp");

		for (i32 i = 0; i < bspLoader.m_numleafs; i++)
		{
			printf("Reading bspLeaf %i from total %i (%f procent)\n", i, bspLoader.m_numleafs, (100.f * (float)i / float(bspLoader.m_numleafs)));

			bool isValidBrush = false;

			BSPLeaf& leaf = bspLoader.m_dleafs[i];

			for (i32 b = 0; b < leaf.numLeafBrushes; b++)
			{
				AlignedObjectArray<Vec3> planeEquations;

				i32 brushid = bspLoader.m_dleafbrushes[leaf.firstLeafBrush + b];

				BSPBrush& brush = bspLoader.m_dbrushes[brushid];
				if (brush.shaderNum != -1)
				{
					if (bspLoader.m_dshaders[brush.shaderNum].contentFlags & BSPCONTENTS_SOLID)
					{
						brush.shaderNum = -1;

						for (i32 p = 0; p < brush.numSides; p++)
						{
							i32 sideid = brush.firstSide + p;
							BSPBrushSide& brushside = bspLoader.m_dbrushsides[sideid];
							i32 planeid = brushside.planeNum;
							BSPPlane& plane = bspLoader.m_dplanes[planeid];
							Vec3 planeEq;
							planeEq.setVal(
								plane.normal[0],
								plane.normal[1],
								plane.normal[2]);
							planeEq[3] = scaling * -plane.dist;

							planeEquations.push_back(planeEq);
							isValidBrush = true;
						}
						if (isValidBrush)
						{
							AlignedObjectArray<Vec3> vertices;
							GeometryUtil::getVerticesFromPlaneEquations(planeEquations, vertices);

							bool isEntity = false;
							Vec3 entityTarget(0.f, 0.f, 0.f);
							addConvexVerticesCollider(vertices, isEntity, entityTarget);
						}
					}
				}
			}
		}

#define USE_ENTITIES
#ifdef USE_ENTITIES

		{
			i32 i;
			for (i = 0; i < bspLoader.m_num_entities; i++)
			{
				const BSPEntity& entity = bspLoader.m_entities[i];
				tukk cl = bspLoader.getValueForKey(&entity, "classname");
				if (!strcmp(cl, "trigger_push"))
				{
					Vec3 targetLocation(0.f, 0.f, 0.f);

					cl = bspLoader.getValueForKey(&entity, "target");
					if (strcmp(cl, ""))
					{
						//its not empty so ...

						/*
							//lookup the target position for the jumppad:
							const BSPEntity* targetentity = bspLoader.getEntityByValue( "targetname" , cl );
							if (targetentity)
							{
								if (bspLoader.getVectorForKey( targetentity , "origin",&targetLocation[0]))
								{
																	
								}
							}
							*/

						cl = bspLoader.getValueForKey(&entity, "model");
						if (strcmp(cl, ""))
						{
							// add the model as a brush
							if (cl[0] == '*')
							{
								i32 modelnr = atoi(&cl[1]);
								if ((modelnr >= 0) && (modelnr < bspLoader.m_nummodels))
								{
									const BSPModel& model = bspLoader.m_dmodels[modelnr];
									for (i32 n = 0; n < model.numBrushes; n++)
									{
										AlignedObjectArray<Vec3> planeEquations;
										bool isValidBrush = false;

										//convert brush
										const BSPBrush& brush = bspLoader.m_dbrushes[model.firstBrush + n];
										{
											for (i32 p = 0; p < brush.numSides; p++)
											{
												i32 sideid = brush.firstSide + p;
												BSPBrushSide& brushside = bspLoader.m_dbrushsides[sideid];
												i32 planeid = brushside.planeNum;
												BSPPlane& plane = bspLoader.m_dplanes[planeid];
												Vec3 planeEq;
												planeEq.setVal(
													plane.normal[0],
													plane.normal[1],
													plane.normal[2]);
												planeEq[3] = scaling * -plane.dist;
												planeEquations.push_back(planeEq);
												isValidBrush = true;
											}
											if (isValidBrush)
											{
												AlignedObjectArray<Vec3> vertices;
												GeometryUtil::getVerticesFromPlaneEquations(planeEquations, vertices);

												bool isEntity = true;
												addConvexVerticesCollider(vertices, isEntity, targetLocation);
											}
										}
									}
								}
							}
							else
							{
								printf("unsupported trigger_push model, md3 ?\n");
							}
						}
					}
				}
			}
		}

#endif  //USE_ENTITIES

		//progressEnd();
	}
}
