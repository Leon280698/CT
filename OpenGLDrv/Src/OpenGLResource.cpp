#include "OpenGLResource.h"

#include "../Inc/OpenGLRenderDevice.h"

FOpenGLResource::FOpenGLResource(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : RenDev(InRenDev),
                                                                                   CacheId(InCacheId),
                                                                                   Revision(-1),
                                                                                   HashIndex(INDEX_NONE),
                                                                                   HashNext(NULL){
	RenDev->AddResource(this);
}

FOpenGLResource::~FOpenGLResource(){
	RenDev->RemoveResource(this);
}

// FOpenGLShader

FOpenGLShader::FOpenGLShader(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                               Program(GL_NONE){}

FOpenGLShader::~FOpenGLShader(){
	if(Program)
		glDeleteProgram(Program);
}

void FOpenGLShader::Cache(FShaderGLSL* Shader){
	GLuint VertexShader = CompileShader(Shader, GL_VERTEX_SHADER);
	GLuint FragmentShader = CompileShader(Shader, GL_FRAGMENT_SHADER);

	// Set revision even if compilation is unsuccessful to avoid recompiling the invalid shader each time it is set
	Revision = Shader->GetRevision();

	if(!VertexShader || !FragmentShader){
		if(VertexShader)
			glDeleteShader(VertexShader);

		if(FragmentShader)
			glDeleteShader(FragmentShader);

		return;
	}

	GLuint NewProgram = glCreateProgram();

	glAttachShader(NewProgram, VertexShader);
	glAttachShader(NewProgram, FragmentShader);
	glLinkProgram(NewProgram);
	glDetachShader(NewProgram, VertexShader);
	glDetachShader(NewProgram, FragmentShader);
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);

	GLint Status;

	glGetProgramiv(NewProgram, GL_LINK_STATUS, &Status);

	if(!Status){
		GLchar Buffer[512];

		glGetProgramInfoLog(NewProgram, ARRAY_COUNT(Buffer), NULL, Buffer);
		debugf("Shader program linking failed for %s: %s", Shader->GetName(), Buffer);
		glDeleteProgram(NewProgram);
	}else{
		if(Program)
			glDeleteProgram(Program);

		Program = NewProgram;
	}
}

void FOpenGLShader::Bind() const{
	checkSlow(Program);
	glUseProgram(Program);
}

GLuint FOpenGLShader::CompileShader(FShaderGLSL* Shader, GLenum Type){
	GLuint Handle = glCreateShader(Type);
	const TCHAR* ShaderText = NULL;
	const TCHAR* FileExt = NULL;

	if(Type == GL_VERTEX_SHADER){
		ShaderText = Shader->GetVertexShaderText();
		FileExt = VERTEX_SHADER_FILE_EXTENSION;
	}else if(Type == GL_FRAGMENT_SHADER){
		ShaderText = Shader->GetFragmentShaderText();
		FileExt = FRAGMENT_SHADER_FILE_EXTENSION;
	}else{
		appErrorf("Unsupported shader type (%i)", Type);
	}

	glShaderSource(Handle, 1, &ShaderText, NULL);
	glCompileShader(Handle);

	GLint Status;

	glGetShaderiv(Handle, GL_COMPILE_STATUS, &Status);

	if(!Status){
		GLchar Buffer[512];

		glGetShaderInfoLog(Handle, ARRAY_COUNT(Buffer), NULL, Buffer);
		debugf("Shader compilation failed for %s%s: %s", Shader->GetName(), FileExt, Buffer);
		glDeleteShader(Handle);
		Handle = GL_NONE;
	}

	return Handle;
}

// FOpenGLIndexBuffer

FOpenGLIndexBuffer::FOpenGLIndexBuffer(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic) : FOpenGLResource(InRenDev, InCacheId),
                                                                                                           EBO(GL_NONE),
                                                                                                           IndexSize(0),
                                                                                                           BufferSize(0),
                                                                                                           IsDynamic(InIsDynamic){}

FOpenGLIndexBuffer::~FOpenGLIndexBuffer(){
	Free();
}

void FOpenGLIndexBuffer::Cache(FIndexBuffer* IndexBuffer){
	if(!IsDynamic)
		Free();

	if(!EBO)
		glCreateBuffers(1, &EBO);

	INT NewBufferSize = IndexBuffer->GetSize();
	void* Data = RenDev->GetScratchBuffer(NewBufferSize);

	IndexBuffer->GetContents(Data);

	if(IsDynamic){
		if(BufferSize < NewBufferSize){
			BufferSize = NewBufferSize * 2;
			glNamedBufferData(EBO, BufferSize, NULL, GL_DYNAMIC_DRAW);
		}

		glNamedBufferSubData(EBO, 0, NewBufferSize, Data);
	}else{
		BufferSize = NewBufferSize;
		glNamedBufferStorage(EBO, NewBufferSize, Data, 0);
	}

	IndexSize = IndexBuffer->GetIndexSize();
	Revision = IndexBuffer->GetRevision();
}

void FOpenGLIndexBuffer::Free(){
	if(EBO){
		glDeleteBuffers(1, &EBO);
		EBO = 0;
	}
}

void FOpenGLIndexBuffer::Bind() const{
	checkSlow(EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
}

// FOpenGLVertexStream

FOpenGLVertexStream::FOpenGLVertexStream(UOpenGLRenderDevice* InRenDev, QWORD InCacheId, bool InIsDynamic) : FOpenGLResource(InRenDev, InCacheId),
                                                                                                             VBO(GL_NONE),
                                                                                                             Stride(0),
                                                                                                             BufferSize(0),
                                                                                                             IsDynamic(InIsDynamic){}

FOpenGLVertexStream::~FOpenGLVertexStream(){
	Free();
}

void FOpenGLVertexStream::Cache(FVertexStream* VertexStream){
	if(!IsDynamic){
		Free();
		IsDynamic = VertexStream->HintDynamic() != 0;
	}

	if(!VBO)
		glCreateBuffers(1, &VBO);

	INT NewBufferSize = VertexStream->GetSize();
	void* Data = NULL;

	VertexStream->GetRawStreamData(&Data, 0);

	if(!Data){
		Data = RenDev->GetScratchBuffer(NewBufferSize);
		VertexStream->GetStreamData(Data);
	}

	if(IsDynamic){
		if(BufferSize < NewBufferSize){
			BufferSize = NewBufferSize * 2;
			glNamedBufferData(VBO, BufferSize, NULL, GL_DYNAMIC_DRAW);
		}

		glNamedBufferSubData(VBO, 0, NewBufferSize, Data);
	}else{
		BufferSize = NewBufferSize;
		glNamedBufferStorage(VBO, BufferSize, Data, 0);
	}

	Stride = VertexStream->GetStride();
	Revision = VertexStream->GetRevision();
}

void FOpenGLVertexStream::Free(){
	if(VBO){
		glDeleteBuffers(1, &VBO);
		VBO = 0;
	}
}

void FOpenGLVertexStream::Bind(GLuint BindingIndex) const{
	checkSlow(VBO);
	glBindVertexBuffer(BindingIndex, VBO, 0, Stride);
}

// FOpenGLTexture

FSolidColorTexture FOpenGLTexture::ErrorTexture(FColor(255, 0, 255));

FOpenGLTexture::FOpenGLTexture(UOpenGLRenderDevice* InRenDev, QWORD InCacheId) : FOpenGLResource(InRenDev, InCacheId),
                                                                                 Width(0),
                                                                                 Height(0),
                                                                                 TextureHandle(GL_NONE),
                                                                                 FBO(GL_NONE),
                                                                                 DepthStencilAttachment(GL_NONE),
                                                                                 IsCubemap(false),
                                                                                 HasSharedDepthStencil(false){}

FOpenGLTexture::~FOpenGLTexture(){
	Free();
}

void FOpenGLTexture::Cache(FBaseTexture* BaseTexture, bool RenderTargetMatchBackbuffer){
	Free();

	FTexture* Texture = BaseTexture->GetTextureInterface();
	FCubemap* Cubemap = BaseTexture->GetCubemapInterface();
	FRenderTarget* RenderTarget = BaseTexture->GetRenderTargetInterface();
	ETextureFormat SrcFormat = BaseTexture->GetFormat();
	ETextureFormat DestFormat = IsDXTC(SrcFormat) ? SrcFormat : TEXF_RGBA8;

	Width = BaseTexture->GetWidth();
	Height = BaseTexture->GetHeight();
	IsCubemap = false;

	if(Cubemap){
		IsCubemap = true;

		for(INT FaceIndex = 0; FaceIndex < 6; ++FaceIndex){
			FTexture* Face = Cubemap->GetFace(FaceIndex);

			if(!Face)
				Face = &ErrorTexture;

			void* Data = ConvertTextureData(Face, DestFormat, Width, Height, 0);

			UploadTextureData(GL_TEXTURE_CUBE_MAP, DestFormat, Data, Width, Height, Face->GetNumMips(), FaceIndex);
			Face->UnloadRawTextureData(0);
		}

		if(TextureHandle)
			glGenerateTextureMipmap(TextureHandle);
	}else if(Texture){
		void* Data = ConvertTextureData(Texture, DestFormat, Width, Height, 0);

		UploadTextureData(GL_TEXTURE_2D, DestFormat, Data, Width, Height, Texture->GetNumMips());
		glGenerateTextureMipmap(TextureHandle);
		Texture->UnloadRawTextureData(0);
	}else if(RenderTarget){
		if(Width == 0 || Height == 0)
			return;

		GLenum Format;

		if(RenderTargetMatchBackbuffer)
			Format = RenDev->Use16bit ? GL_RGB565 : GL_RGB8;
		else
			Format = GL_RGBA8;

		glCreateFramebuffers(1, &FBO);
		glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);
		glTextureStorage2D(TextureHandle, 1, Format, Width, Height);
		glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, TextureHandle, 0);

		// NOTE: This is a special case: The backbuffer and FrameFX working target need to share their depth and stencil buffers
		HasSharedDepthStencil = RenderTarget == &RenDev->Backbuffer || RenderTarget == UFrameFX::WorkingTarget;

		if(HasSharedDepthStencil){
			checkSlow(RenDev->BackbufferDepthStencil != GL_NONE);
			DepthStencilAttachment = RenDev->BackbufferDepthStencil;
		}else{
			bool IsMipTarget = false;

			for(INT i = 0; i < UFrameFX::MaxMips; ++i){
				if(RenderTarget == UFrameFX::MipTargets[i]){
					IsMipTarget = true;

					break;
				}
			}

			if(IsMipTarget){ // The FrameFX Mip targets don't need a depth and stencil buffer
				DepthStencilAttachment = GL_NONE;
			}else{
				glCreateRenderbuffers(1, &DepthStencilAttachment);
				glNamedRenderbufferStorage(DepthStencilAttachment, GL_DEPTH24_STENCIL8, Width, Height);
			}
		}

		if(DepthStencilAttachment)
			glNamedFramebufferRenderbuffer(FBO, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, DepthStencilAttachment);

		checkSlow(glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	Revision = BaseTexture->GetRevision();
}

void FOpenGLTexture::Free(){
	Width = 0;
	Height = 0;

	if(TextureHandle){
		glDeleteTextures(1, &TextureHandle);
		TextureHandle = GL_NONE;
	}

	if(FBO){
		glDeleteFramebuffers(1, &FBO);
		FBO = GL_NONE;
	}

	if(DepthStencilAttachment && !HasSharedDepthStencil){
		glDeleteRenderbuffers(1, &DepthStencilAttachment);
		DepthStencilAttachment = GL_NONE;
	}
}

void FOpenGLTexture::BindTexture(GLuint TextureUnit){
	glBindTextureUnit(TextureUnit, TextureHandle);
}

void FOpenGLTexture::BindRenderTarget(){
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
}

void FOpenGLTexture::UploadTextureData(GLenum Target, ETextureFormat Format, void* Data, INT Width, INT Height, INT NumMips, INT CubemapFace){
	if(!Data || Width == 0 || Height == 0){
		Format = TEXF_RGBA8;
		Data = ErrorTexture.GetRawTextureData(0);
		Width = ErrorTexture.GetWidth();
		Height = ErrorTexture.GetHeight();
		NumMips = 1;
	}

	if(IsDXTC(Format)){
		GLenum GLFormat;

		if(Format == TEXF_DXT1)
			GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		else if(Format == TEXF_DXT3)
			GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		else
			GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

		if(Target == GL_TEXTURE_CUBE_MAP){
			checkSlow(CubemapFace >= 0 && CubemapFace < 6);

			if(!TextureHandle)
				glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &TextureHandle);

			Target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + CubemapFace;
		}else{
			checkSlow(!TextureHandle);
			glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);
		}

		glCompressedTextureImage2DEXT(TextureHandle, Target, 0, GLFormat, Width, Height, 0, GetBytesPerPixel(Format, Width * Height), Data);
	}else{
		GLenum GLFormat = GL_NONE;
		GLenum GLType = GL_NONE;

		if(Format == TEXF_RGBA8){
			GLFormat = GL_BGRA;
			GLType = GL_UNSIGNED_BYTE;
		}else{
			appErrorf("Invalid texture format '%i'", Format);
		}

		if(Target == GL_TEXTURE_2D){
			checkSlow(!TextureHandle);
			glCreateTextures(GL_TEXTURE_2D, 1, &TextureHandle);
			glTextureStorage2D(TextureHandle, NumMips, GL_RGBA8, Width, Height);
			glTextureSubImage2D(TextureHandle, 0, 0, 0, Width, Height, GLFormat, GLType, Data);
		}else if(Target == GL_TEXTURE_CUBE_MAP){
			if(!TextureHandle){
				glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &TextureHandle);
				glTextureStorage2D(TextureHandle, NumMips, GL_RGBA8, Width, Height);
			}

			glTextureSubImage3D(TextureHandle, 0, 0, 0, CubemapFace, Width, Height, 1, GLFormat, GLType, Data);
		}else{
			appErrorf("Invalid texture target '%x'", Target);
		}
	}
}

void* FOpenGLTexture::ConvertTextureData(FTexture* Texture, ETextureFormat DestFormat, INT Width, INT Height, INT MipIndex){
	void* Result;
	ETextureFormat SrcFormat = Texture->GetFormat();
	INT NumPixels = Width * Height;
	INT SrcBufferSize = GetBytesPerPixel(SrcFormat, NumPixels);
	INT DestBufferSize = GetBytesPerPixel(DestFormat, NumPixels);
	void* TextureData = Texture->GetRawTextureData(MipIndex);

	if(TextureData){
		Result = RenDev->GetScratchBuffer(DestBufferSize);
	}else{
		TextureData = RenDev->GetScratchBuffer(SrcBufferSize + DestBufferSize);
		Result = static_cast<BYTE*>(TextureData) + SrcBufferSize;
		Texture->GetTextureData(MipIndex, TextureData, 0, SrcFormat);
	}

	if(SrcFormat == DestFormat)
		return TextureData;

	if(DestFormat == TEXF_RGBA8){
		if(SrcFormat == TEXF_P8){
			UTexture* Tex = Texture->GetUTexture();
			check(Tex && Tex->Palette);
			const TArray<FColor>& Palette = Tex->Palette->Colors;

			for(INT i = 0; i < NumPixels; ++i)
				static_cast<FColor*>(Result)[i] = Palette[static_cast<BYTE*>(TextureData)[i]];
		}else if(SrcFormat == TEXF_RGB8){
			for(INT i = 0; i < NumPixels; ++i){
				BYTE* RGB = static_cast<BYTE*>(TextureData) + i * 3;
				static_cast<FColor*>(Result)[i] = FColor(RGB[0], RGB[1], RGB[2]);
			}
		}else if(SrcFormat == TEXF_L8){
			for(INT i = 0; i < NumPixels; ++i){
				BYTE Value = static_cast<BYTE*>(TextureData)[i];
				static_cast<FColor*>(Result)[i] = FColor(Value, Value, Value);
			}
		}else if(SrcFormat == TEXF_G16){
			for(INT i = 0; i < NumPixels; ++i){
				BYTE Intensity = static_cast<_WORD*>(TextureData)[i] >> 8;
				static_cast<FColor*>(Result)[i] = FColor(Intensity, Intensity, Intensity);
			}
		}else if(SrcFormat == TEXF_V8U8){
			ConvertV8U8ToRGBA8(TextureData, Result, Width, Height);
		}else if(SrcFormat == TEXF_L6V5U5){
			ConvertL6V5U5ToRGBA8(TextureData, Result, Width, Height);
		}else if(SrcFormat == TEXF_X8L8V8U8){
			ConvertX8L8V8U8ToRGB8(TextureData, Result, Width, Height);
		}else{
			Result = NULL;
		}
	}else{
		Result = NULL;
	}

	return Result;
}
