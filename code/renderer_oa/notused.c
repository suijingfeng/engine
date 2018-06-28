/*
===================
ParseStageSimple


leilei - the purpose of this is to load textures after processing their blending properties,
	 so we can approximate effects on incapable hardware such as Matrox Mystique, S3 ViRGE,
	 PowerVR PCX2, software rendering...
	 A lot of things are stripped out (like GLSL and multitexture stuff)
===================
*/

 
qboolean ParseStageSimple( shaderStage_t *stage, char **text )
{
	char *token;
	char programName[MAX_QPATH];
	char programFragmentObjects[MAX_PROGRAM_OBJECTS][MAX_QPATH];
	char imageName[MAX_QPATH];	// for loading later
	char imageNameAnim0[MAX_QPATH];	
	char imageNameAnim1[MAX_QPATH];	
	char imageNameAnim2[MAX_QPATH];	
	char imageNameAnim3[MAX_QPATH];	
	char imageNameAnim4[MAX_QPATH];	
	char imageNameAnim5[MAX_QPATH];	
	char imageNameAnim6[MAX_QPATH];	
	char imageNameAnim7[MAX_QPATH];	
	char imageNameAnim8[MAX_QPATH];	
	imgType_t itype = IMGTYPE_COLORALPHA;
	imgFlags_t iflags = IMGFLAG_NONE;
	int numVertexObjects = 0;
	int numFragmentObjects = 0;
	int loadlater = 0;
	int depthMaskBits = GLS_DEPTHMASK_TRUE, blendSrcBits = 0, blendDstBits = 0, atestBits = 0, depthFuncBits = 0;
	qboolean depthMaskExplicit = qfalse;

	qboolean stageMipmaps = !shader.noMipMaps;
	stage->active = qtrue;
	memset(programName, 0, sizeof(programName));



	while ( 1 )
	{
		//stage->isBlend = 0;
		token = COM_ParseExt( text, qtrue );
		if ( !token[0] )
		{
			ri.Printf( PRINT_WARNING, "WARNING: no matching '}' found\n" );
			return qfalse;
		}

		if ( token[0] == '}' )
	        break;
		//
		// program <name>
		//
		else if (!Q_stricmp(token, "program")) {
			token = COM_ParseExt(text, qfalse);

				continue;

			if (!token[0]) {
				ri.Printf(PRINT_WARNING, "WARNING: missing parameter for 'program' keyword in shader '%s'\n", shader.name);
				return qfalse;
			}

			Q_strncpyz(programName, token, sizeof(programName));
		}
		//
		// vertexProgram <path1> .... <pathN>
		//
		else if (!Q_stricmp(token, "vertexProgram")) {
			token = COM_ParseExt(text, qfalse);


				while (token[0])
					token = COM_ParseExt(text, qfalse);

				continue;


			if (!token[0]) {
				ri.Printf(PRINT_WARNING, "WARNING: missing parameter(s) for 'vertexProgram' keyword in shader '%s'\n", shader.name);
				return qfalse;
			}

			// parse up to MAX_PROGRAM_OBJECTS files
			for(;;) {
				if (numVertexObjects < MAX_PROGRAM_OBJECTS) {
					numVertexObjects++;
				} else {
					ri.Printf(PRINT_WARNING, "WARNING: Too many parameters for 'vertexProgram' keyword in shader '%s'\n", shader.name);
					return qfalse;
				}

				token = COM_ParseExt(text, qfalse);
				if (!token[0])
					break;
			}
		}
		//
		// fragmentProgram <path1> .... <pathN>
		//
		else if (!Q_stricmp(token, "fragmentProgram")) {
			token = COM_ParseExt(text, qfalse);


				while (token[0])
					token = COM_ParseExt(text, qfalse);

				continue;


			if (!token[0]) {
				ri.Printf(PRINT_WARNING, "WARNING: missing parameter(s) for 'fragmentProgram' keyword in shader '%s'\n", shader.name);
				return qfalse;
			}

			// parse up to MAX_PROGRAM_OBJECTS files
			for(;;) {
				if (numFragmentObjects < MAX_PROGRAM_OBJECTS) {
					Q_strncpyz(programFragmentObjects[numFragmentObjects], token, sizeof(programFragmentObjects[numFragmentObjects]));
					numFragmentObjects++;
				} else {
					ri.Printf(PRINT_WARNING, "WARNING: Too many parameters for 'fragmentProgram' keyword in shader '%s'\n", shader.name);
					return qfalse;
				}

				token = COM_ParseExt(text, qfalse);
				if (!token[0])
					break;
			}
		}
		else if ( !Q_stricmp( token, "mipOffset" ) ){
			token = COM_ParseExt(text,qfalse);
			stage->mipBias = atoi(token);
		}
		else if ( !Q_stricmp( token, "nomipmaps" ) ){
			stageMipmaps = qfalse;
		}
		//
		// map <name>
		//
		else if ( !Q_stricmp( token, "map" ) )
		{
			token = COM_ParseExt( text, qfalse );
			if ( !token[0] )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameter for 'map' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}

			if ( !Q_stricmp( token, "$whiteimage" ) )
			{
				stage->bundle[0].image[0] = tr.whiteImage;
				continue;
			}
			else if ( !Q_stricmp( token, "$lightmap" ) )
			{
				stage->bundle[0].isLightmap = qtrue;
				if ( shader.lightmapIndex < 0 || !tr.lightmaps ) {
					stage->bundle[0].image[0] = tr.whiteImage;
				} else {
					stage->bundle[0].image[0] = tr.lightmaps[shader.lightmapIndex];
				}
				continue;
			}
			else
			{
				imgType_t type = IMGTYPE_COLORALPHA;
				imgFlags_t flags = IMGFLAG_NONE;

				if (!shader.noMipMaps)
					flags |= IMGFLAG_MIPMAP;

				if (!shader.noPicMip)
					flags |= IMGFLAG_PICMIP;

				//stage->bundle[0].image[0] = R_FindImageFile( token, type, flags );
				stage->bundle[0].image[0] = tr.whiteImage;
				COM_StripExtension( token, imageName, MAX_QPATH );
				itype = type; iflags = flags;
				loadlater = 1;
//				imageName = va("%s",token);

	//			if ( !stage->bundle[0].image[0] )
	//			{
	//				ri.Printf( PRINT_WARNING, "WARNING: R_FindImageFile could not find '%s' in shader '%s'\n", token, shader.name );
	//				return qfalse;
	//			}
			}
		}
		//
		// clampmap <name>
		//
		else if ( !Q_stricmp( token, "clampmap" ) )
		{
			imgType_t type = IMGTYPE_COLORALPHA;
			imgFlags_t flags = IMGFLAG_CLAMPTOEDGE;

			token = COM_ParseExt( text, qfalse );
			if ( !token[0] )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameter for 'clampmap' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}

			if (!shader.noMipMaps)
				flags |= IMGFLAG_MIPMAP;

			if (!shader.noPicMip)
				flags |= IMGFLAG_PICMIP;

			stage->bundle[0].image[0] = tr.whiteImage;
			COM_StripExtension( token, imageName, MAX_QPATH );
			itype = type; iflags = flags;
			loadlater = 1;
		}
		//
		// animMap <frequency> <image1> .... <imageN>
		//
		else if ( !Q_stricmp( token, "animMap" ) )
		{
			token = COM_ParseExt( text, qfalse );
			if ( !token[0] )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameter for 'animMmap' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}
			stage->bundle[0].imageAnimationSpeed = atof( token );

			// parse up to MAX_IMAGE_ANIMATIONS animations
			while ( 1 ) {
				int		num;

				token = COM_ParseExt( text, qfalse );
				if ( !token[0] ) {
					break;
				}
				num = stage->bundle[0].numImageAnimations;
				if ( num < MAX_IMAGE_ANIMATIONS ) {
					imgFlags_t flags = IMGFLAG_NONE;

					if (!shader.noMipMaps)
						flags |= IMGFLAG_MIPMAP;

					if (!shader.noPicMip)
						flags |= IMGFLAG_PICMIP;

					//stage->bundle[0].image[num] = R_FindImageFile( token, IMGTYPE_COLORALPHA, flags );
					stage->bundle[0].image[num] = tr.whiteImage;
					if(num == 0) COM_StripExtension( token, imageNameAnim0, MAX_QPATH );
					if(num == 1) COM_StripExtension( token, imageNameAnim1, MAX_QPATH );
					if(num == 2) COM_StripExtension( token, imageNameAnim2, MAX_QPATH );
					if(num == 3) COM_StripExtension( token, imageNameAnim3, MAX_QPATH );
					if(num == 4) COM_StripExtension( token, imageNameAnim4, MAX_QPATH );
					if(num == 5) COM_StripExtension( token, imageNameAnim5, MAX_QPATH );
					if(num == 6) COM_StripExtension( token, imageNameAnim6, MAX_QPATH );
					if(num == 7) COM_StripExtension( token, imageNameAnim7, MAX_QPATH );
					if(num == 8) COM_StripExtension( token, imageNameAnim8, MAX_QPATH );
					iflags = flags;
					loadlater = 1;
					stage->bundle[0].numImageAnimations++;
				}
			}
		}
		else if ( !Q_stricmp( token, "clampAnimMap" ) )
		{
			token = COM_ParseExt( text, qfalse );
			if ( !token[0] )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameter for 'clampAnimMmap' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}
			stage->bundle[0].imageAnimationSpeed = atof( token );

			// parse up to MAX_IMAGE_ANIMATIONS animations
			while ( 1 ) {
				int		num;

				token = COM_ParseExt( text, qfalse );
				if ( !token[0] ) {
					break;
				}
				num = stage->bundle[0].numImageAnimations;
				if ( num < MAX_IMAGE_ANIMATIONS ) {
					imgFlags_t flags = IMGFLAG_GENNORMALMAP;

					if (stageMipmaps)
						flags |= IMGFLAG_MIPMAP;

					if (!shader.noPicMip)
						flags |= IMGFLAG_PICMIP;

					stage->bundle[0].image[num] = R_FindImageFile( token, IMGTYPE_COLORALPHA, flags );

					if ( !stage->bundle[0].image[num] )
					{
						ri.Printf( PRINT_WARNING, "WARNING: R_FindImageFile could not find '%s' in shader '%s'\n", token, shader.name );
						return qfalse;
					}
					stage->bundle[0].numImageAnimations++;
				}
			}
		}
		else if ( !Q_stricmp( token, "videoMap" ) )
		{
			token = COM_ParseExt( text, qfalse );
			if ( !token[0] )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameter for 'videoMmap' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}
			stage->bundle[0].videoMapHandle = ri.CIN_PlayCinematic( token, 0, 0, 256, 256, (CIN_loop | CIN_silent | CIN_shader));
			if (stage->bundle[0].videoMapHandle != -1) {
				stage->bundle[0].isVideoMap = qtrue;
				stage->bundle[0].image[0] = tr.scratchImage[stage->bundle[0].videoMapHandle];
			}
		}
		//
		// alphafunc <func>
		//
		else if ( !Q_stricmp( token, "alphaFunc" ) )
		{
			token = COM_ParseExt( text, qfalse );
			if ( !token[0] )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameter for 'alphaFunc' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}

			atestBits = NameToAFunc( token );
		}
		//
		// depthFunc <func>
		//
		else if ( !Q_stricmp( token, "depthfunc" ) )
		{
			token = COM_ParseExt( text, qfalse );

			if ( !token[0] )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameter for 'depthfunc' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}

			if ( !Q_stricmp( token, "lequal" ) )
			{
				depthFuncBits = 0;
			}
			else if ( !Q_stricmp( token, "equal" ) )
			{
				depthFuncBits = GLS_DEPTHFUNC_EQUAL;
			}
			else
			{
				ri.Printf( PRINT_WARNING, "WARNING: unknown depthfunc '%s' in shader '%s'\n", token, shader.name );
				continue;
			}
		}
		//
		// detail
		//
		else if ( !Q_stricmp( token, "detail" ) )
		{
			stage->isDetail = qtrue;
		}
		//
		// blendfunc <srcFactor> <dstFactor>
		// or blendfunc <add|filter|blend>
		//
		else if ( !Q_stricmp( token, "blendfunc" ) )
		{
			token = COM_ParseExt( text, qfalse );
			if ( token[0] == 0 )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parm for blendFunc in shader '%s'\n", shader.name );
				continue;
			}
			// check for "simple" blends first
			if ( !Q_stricmp( token, "add" ) ) {
				blendSrcBits = GLS_SRCBLEND_ONE;
				blendDstBits = GLS_DSTBLEND_ONE;
			} else if ( !Q_stricmp( token, "filter" ) ) {
				blendSrcBits = GLS_SRCBLEND_DST_COLOR;
				blendDstBits = GLS_DSTBLEND_ZERO;
			} else if ( !Q_stricmp( token, "blend" ) ) {
				blendSrcBits = GLS_SRCBLEND_SRC_ALPHA;
				blendDstBits = GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
				
			} else {
				// complex double blends
				blendSrcBits = NameToSrcBlendMode( token );
				token = COM_ParseExt( text, qfalse );
				if ( token[0] == 0 )
				{
					ri.Printf( PRINT_WARNING, "WARNING: missing parm for blendFunc in shader '%s'\n", shader.name );
					continue;
				}
				blendDstBits = NameToDstBlendMode( token );
			}
			
			stage->isBlend = 1;	// 2x2

			// clear depth mask for blended surfaces
			if ( !depthMaskExplicit )
			{
				depthMaskBits = 0;
			}


		}
		//
		// rgbGen
		//
		else if ( !Q_stricmp( token, "rgbGen" ) )
		{
			token = COM_ParseExt( text, qfalse );
			if ( token[0] == 0 )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameters for rgbGen in shader '%s'\n", shader.name );
				continue;
			}

			if ( !Q_stricmp( token, "wave" ) )
			{
				ParseWaveForm( text, &stage->rgbWave );
				stage->rgbGen = CGEN_WAVEFORM;
			}
			else if ( !Q_stricmp( token, "const" ) )
			{
				vec3_t	color;

				ParseVector( text, 3, color );
				stage->constantColor[0] = 255 * color[0];
				stage->constantColor[1] = 255 * color[1];
				stage->constantColor[2] = 255 * color[2];

				stage->rgbGen = CGEN_CONST;
			}
			else if ( !Q_stricmp( token, "identity" ) )
			{
				stage->rgbGen = CGEN_IDENTITY;
			}
			else if ( !Q_stricmp( token, "identityLighting" ) )
			{
				stage->rgbGen = CGEN_IDENTITY_LIGHTING;
			}
			else if ( !Q_stricmp( token, "entity" ) )
			{
				stage->rgbGen = CGEN_ENTITY;
			}
			else if ( !Q_stricmp( token, "oneMinusEntity" ) )
			{
				stage->rgbGen = CGEN_ONE_MINUS_ENTITY;
			}
			else if ( !Q_stricmp( token, "vertex" ) )
			{
				stage->rgbGen = CGEN_VERTEX;
				if ( stage->alphaGen == 0 ) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if ( !Q_stricmp( token, "exactVertex" ) )
			{
				stage->rgbGen = CGEN_EXACT_VERTEX;
			}
			else if ( !Q_stricmp( token, "vertexLighting" ) )	// leilei - vertex WITH a lighting pass after
			{
				stage->rgbGen = CGEN_VERTEX_LIT;
				if ( stage->alphaGen == 0 ) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if ( !Q_stricmp( token, "vertexLighting2" ) )	// leilei - second vertex color
			{
				stage->rgbGen = CGEN_VERTEX_LIT;
				if ( stage->alphaGen == 0 ) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if ( !Q_stricmp( token, "lightingDiffuse" ) )
			{
				stage->rgbGen = CGEN_LIGHTING_DIFFUSE;
			}
			else if ( !Q_stricmp( token, "lightingUniform" ) )
			{
				stage->rgbGen = CGEN_LIGHTING_UNIFORM;
			}
			else if ( !Q_stricmp( token, "lightingDynamic" ) )
			{
				stage->rgbGen = CGEN_LIGHTING_DYNAMIC;
			}
			else if ( !Q_stricmp( token, "flatAmbient" ) )
			{
				stage->rgbGen = CGEN_LIGHTING_FLAT_AMBIENT;
			}
			else if ( !Q_stricmp( token, "flatDirect" ) )
			{
				stage->rgbGen = CGEN_LIGHTING_FLAT_DIRECT;
			}
			else if ( !Q_stricmp( token, "oneMinusVertex" ) )
			{
				stage->rgbGen = CGEN_ONE_MINUS_VERTEX;
			}
			else if ( !Q_stricmp( token, "lightingSpecularDiffuse" ) )	// leilei - deprecated
			{
				stage->rgbGen = CGEN_LIGHTING_DIFFUSE;
			}
			else
			{
				ri.Printf( PRINT_WARNING, "WARNING: unknown rgbGen parameter '%s' in shader '%s'\n", token, shader.name );
				continue;
			}
		}
		//
		// alphaGen 
		//
		else if ( !Q_stricmp( token, "alphaGen" ) )
		{
			token = COM_ParseExt( text, qfalse );
			if ( token[0] == 0 )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing parameters for alphaGen in shader '%s'\n", shader.name );
				continue;
			}

			if ( !Q_stricmp( token, "wave" ) )
			{
				ParseWaveForm( text, &stage->alphaWave );
				stage->alphaGen = AGEN_WAVEFORM;
			}
			else if ( !Q_stricmp( token, "const" ) )
			{
				token = COM_ParseExt( text, qfalse );
				stage->constantColor[3] = 255 * atof( token );
				stage->alphaGen = AGEN_CONST;
			}
			else if ( !Q_stricmp( token, "identity" ) )
			{
				stage->alphaGen = AGEN_IDENTITY;
			}
			else if ( !Q_stricmp( token, "entity" ) )
			{
				stage->alphaGen = AGEN_ENTITY;
			}
			else if ( !Q_stricmp( token, "oneMinusEntity" ) )
			{
				stage->alphaGen = AGEN_ONE_MINUS_ENTITY;
			}
			else if ( !Q_stricmp( token, "vertex" ) )
			{
				stage->alphaGen = AGEN_VERTEX;
			}
			else if ( !Q_stricmp( token, "lightingSpecular" ) )
			{
				stage->alphaGen = AGEN_LIGHTING_SPECULAR;
			}
			else if ( !Q_stricmp( token, "oneMinusVertex" ) )
			{
				stage->alphaGen = AGEN_ONE_MINUS_VERTEX;
			}
			else if ( !Q_stricmp( token, "portal" ) )
			{
				stage->alphaGen = AGEN_PORTAL;
				token = COM_ParseExt( text, qfalse );
				if ( token[0] == 0 )
				{
					shader.portalRange = 256;
					ri.Printf( PRINT_WARNING, "WARNING: missing range parameter for alphaGen portal in shader '%s', defaulting to 256\n", shader.name );
				}
				else
				{
					shader.portalRange = atof( token );
				}
			}
			else
			{
				ri.Printf( PRINT_WARNING, "WARNING: unknown alphaGen parameter '%s' in shader '%s'\n", token, shader.name );
				continue;
			}
		}
		//
		// tcGen <function>
		//
		else if ( !Q_stricmp(token, "texgen") || !Q_stricmp( token, "tcGen" ) ) 
		{
			token = COM_ParseExt( text, qfalse );
			if ( token[0] == 0 )
			{
				ri.Printf( PRINT_WARNING, "WARNING: missing texgen parm in shader '%s'\n", shader.name );
				continue;
			}

			if ( !Q_stricmp( token, "environment" ) )
			{
				stage->bundle[0].tcGen = TCGEN_ENVIRONMENT_MAPPED;
			}
			else if ( !Q_stricmp( token, "cel" ) )
			{
				stage->bundle[0].tcGen = TCGEN_ENVIRONMENT_CELSHADE_MAPPED;
			}
			else if ( !Q_stricmp( token, "celshading" ) )		// leilei - my technique is different
			{
				stage->bundle[0].tcGen = TCGEN_ENVIRONMENT_CELSHADE_LEILEI;
			}
			else if ( !Q_stricmp( token, "environmentWater" ) )
			{
				stage->bundle[0].tcGen = TCGEN_ENVIRONMENT_MAPPED_WATER;	// leilei - water's envmaps
			}
			else if ( !Q_stricmp( token, "lightmap" ) )
			{
				stage->bundle[0].tcGen = TCGEN_LIGHTMAP;
			}
			else if ( !Q_stricmp( token, "texture" ) || !Q_stricmp( token, "base" ) )
			{
				stage->bundle[0].tcGen = TCGEN_TEXTURE;
			}
			else if ( !Q_stricmp( token, "vector" ) )
			{
				ParseVector( text, 3, stage->bundle[0].tcGenVectors[0] );
				ParseVector( text, 3, stage->bundle[0].tcGenVectors[1] );

				stage->bundle[0].tcGen = TCGEN_VECTOR;
			}
			else 
			{
				ri.Printf( PRINT_WARNING, "WARNING: unknown texgen parm in shader '%s'\n", shader.name );
			}
		}
		//
		// tcMod <type> <...>
		//
		else if ( !Q_stricmp( token, "tcMod" ) )
		{
			char buffer[1024] = "";

			while ( 1 )
			{
				token = COM_ParseExt( text, qfalse );
				if ( token[0] == 0 )
					break;
				strcat( buffer, token );
				strcat( buffer, " " );
			}

			ParseTexMod( buffer, stage );

			continue;
		}
		//
		// depthmask
		//
		else if ( !Q_stricmp( token, "depthwrite" ) )
		{
			depthMaskBits = GLS_DEPTHMASK_TRUE;
			depthMaskExplicit = qtrue;

			continue;
		}

		else
		{
			ri.Printf( PRINT_WARNING, "WARNING: unknown parameter '%s' in shader '%s'\n", token, shader.name );
			return qfalse;
		}
	}

	//
	// if cgen isn't explicitly specified, use either identity or identitylighting
	//
	if ( stage->rgbGen == CGEN_BAD ) {
		if ( blendSrcBits == 0 ||
			blendSrcBits == GLS_SRCBLEND_ONE || 
			blendSrcBits == GLS_SRCBLEND_SRC_ALPHA ) {
			stage->rgbGen = CGEN_IDENTITY_LIGHTING;
		} else {
			stage->rgbGen = CGEN_IDENTITY;
		}
	}

	
	//
	// implicitly assume that a GL_ONE GL_ZERO blend mask disables blending
	//
	if ( ( blendSrcBits == GLS_SRCBLEND_ONE ) && 
		 ( blendDstBits == GLS_DSTBLEND_ZERO ) )
	{
		blendDstBits = blendSrcBits = 0;
		depthMaskBits = GLS_DEPTHMASK_TRUE;
	}

	// decide which agens we can skip
	if ( stage->alphaGen == AGEN_IDENTITY ) {
		if ( stage->rgbGen == CGEN_IDENTITY
			|| stage->rgbGen == CGEN_LIGHTING_DIFFUSE 
			|| stage->rgbGen == CGEN_LIGHTING_UNIFORM 
			|| stage->rgbGen == CGEN_LIGHTING_DYNAMIC) {
			stage->alphaGen = AGEN_SKIP;
		}
	}

	//
	// compute state bits
	//
	stage->stateBits = depthMaskBits | 
		               blendSrcBits | blendDstBits | 
					   atestBits | 
					   depthFuncBits;
	//
	// now load our image!
	//

	if (loadlater){


		if (stage->bundle[0].numImageAnimations>0){
		int n, o;
				n= stage->bundle[0].numImageAnimations;
				for (o=0; o<n;o++){
				
					iflags = IMGFLAG_NONE;
					//stage->bundle[0].image[num] = R_FindImageFile( token, IMGTYPE_COLORALPHA, flags );
				
					if(o == 0) COM_StripExtension( imageNameAnim0, imageName, MAX_QPATH );
					if(o == 1) COM_StripExtension( imageNameAnim1, imageName, MAX_QPATH );
					if(o == 2) COM_StripExtension( imageNameAnim2, imageName, MAX_QPATH );
					if(o == 3) COM_StripExtension( imageNameAnim3, imageName, MAX_QPATH );
					if(o == 4) COM_StripExtension( imageNameAnim4, imageName, MAX_QPATH );
					if(o == 5) COM_StripExtension( imageNameAnim5, imageName, MAX_QPATH );
					if(o == 6) COM_StripExtension( imageNameAnim6, imageName, MAX_QPATH );
					if(o == 7) COM_StripExtension( imageNameAnim7, imageName, MAX_QPATH );
					if(o == 8) COM_StripExtension( imageNameAnim8, imageName, MAX_QPATH );
					stage->bundle[0].image[o] = R_FindImageFile( imageName, itype, iflags );
				
				}
		}
		else
		stage->bundle[0].image[0] = R_FindImageFile( imageName, itype, iflags );
		stage->isBlend = 0;

		if (blendSrcBits == GLS_SRCBLEND_SRC_ALPHA &&
		    blendDstBits == GLS_SRCBLEND_ONE) 		// additive, but blended away by alpha
			stage->isBlend = 0;

		}

	return qtrue;
}
