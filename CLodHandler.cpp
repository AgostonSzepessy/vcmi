#include "stdafx.h"
#include "CLodHandler.h"
#include <sstream>
#include <algorithm>
#include <cstring>
#include "boost/filesystem.hpp"   // includes all needed Boost.Filesystem declarations

int readNormalNr (int pos, int bytCon, unsigned char * str)
{
	int ret=0;
	int amp=1;
	if (str)
	{
		for (int i=0; i<bytCon; i++)
		{
			ret+=str[pos+i]*amp;
			amp*=256;
		}
	}
	else return -1;
	return ret;
}
void CPCXConv::openPCX(char * PCX, int len)
{
	pcxs=len;
	pcx=(unsigned char*)PCX;
	/*pcx = new unsigned char[len];
	for (int i=0;i<len;i++)
		pcx[i]=PCX[i];*/
}
void CPCXConv::fromFile(std::string path)
{
	std::ifstream * is = new std::ifstream();
	is -> open(path.c_str(),std::ios::binary);
	is->seekg(0,std::ios::end); // na koniec
	pcxs = is->tellg();  // read length
	is->seekg(0,std::ios::beg); // wracamy na poczatek
	pcx = new unsigned char[pcxs]; // allocate memory 
	is->read((char*)pcx, pcxs); // read map file to buffer
	is->close();
	delete is;
}
void CPCXConv::saveBMP(std::string path)
{
	std::ofstream os;
	os.open(path.c_str(), std::ios::binary);
	os.write((char*)bmp,bmps);
	os.close();
}
void CPCXConv::convert()
{
	BMPHeader bh;
	BMPPalette pal[256];
	Epcxformat format;
	int fSize,i,y;
	bool check1, check2;
	unsigned char add;
	int it=0;

	std::stringstream out;

	fSize = readNormalNr(it,4,pcx);it+=4;
	bh.x = readNormalNr(it,4,pcx);it+=4;
	bh.y = readNormalNr(it,4,pcx);it+=4;
	if (fSize==bh.x*bh.y*3)
		check1=true;
	else 
		check1=false;
	if (fSize==bh.x*bh.y)
		check2=true;
	else 
		check2=false;
	if (check1)
		format=Epcxformat::PCX24B;
	else if (check2)
		format=Epcxformat::PCX8B;
	else 
		return;
	add=(int)(4*(((float)1)-(((float)bh.x/(float)4)-((int)((float)bh.x/(float)4)))));
	if (add==4)
		add=0;
	bh._h3=bh.x*bh.y;
	if (format==Epcxformat::PCX8B)
	{
		bh._c1=0x436;
		bh._c2=0x28;
		bh._c3=1;
		bh._c4=8;
		//bh.dataSize2=bh.dataSize1=maxx*maxy;
		bh.dataSize1=bh.x;
		bh.dataSize2=bh.y;
		bh.fullSize = bh.dataSize1+436;
	}
	else
	{
		bh._c1=0x36;
		bh._c2=0x28;
		bh._c3=1;
		bh._c4=0x18;
		//bh.dataSize2=bh.dataSize1=0xB12;
		bh.dataSize1=bh.x;
		bh.dataSize2=bh.y;
		bh.fullSize=(bh.x+add)*bh.y*3+36+18;
		bh._h3*=3;
	}
	if (format==Epcxformat::PCX8B)
	{
		it = pcxs-256*3;
		for (int i=0;i<256;i++)
		{
			pal[i].R=pcx[it++];
			pal[i].G=pcx[it++];
			pal[i].B=pcx[it++];
			pal[i].F='\0';
		}
	}
	out<<"BM";
	bh.print(out);
	if (format==Epcxformat::PCX8B)
	{
		for (int i=0;i<256;i++)
		{
			out<<pal[i].B;
			out<<pal[i].G;
			out<<pal[i].R;
			out<<pal[i].F;
		}
		for (y=bh.y;y>0;y--)
		{
			it=0xC+(y-1)*bh.x;
			for (int j=0;j<bh.x;j++)
				out<<pcx[it+j];
			if (add>0)
			{
				for (int j=0;j<add;j++)
					out<<'\0'; //bylo z buforu, ale onnie byl incjalizowany (?!)
			}
		}
	}
	else
	{
		for (y=bh.y; y>0; y--)
		{
			it=0xC+(y-1)*bh.x*3;
			for (int j=0;j<bh.x*3;j++)
				out<<pcx[it+j];
			if (add>0)
			{
				for (int j=0;j<add*3;j++)
					out<<'\0'; //bylo z buforu, ale onnie byl incjalizowany (?!)
			}
		}
	}
	std::string temp = out.str();
	bmp = new unsigned char[temp.length()];
	bmps=temp.length();
	for (int a=0;a<temp.length();a++)
	{
		bmp[a]=temp[a];
	}
}

SDL_Surface * CPCXConv::getSurface()
{
	SDL_Surface * ret;
	BMPHeader bh;
	BMPPalette pal[256];
	Epcxformat format;
	int fSize, maxx, maxy,i,y;
	bool check1, check2;
	unsigned char add;
	int it=0;

	fSize = readNormalNr(it,4,pcx);it+=4;
	maxx = readNormalNr(it,4,pcx);it+=4;
	maxy = readNormalNr(it,4,pcx);it+=4;
	if (fSize==maxx*maxy*3)
		check1=true;
	else 
		check1=false;
	if (fSize==maxx*maxy)
		check2=true;
	else 
		check2=false;
	if (check1)
		format=Epcxformat::PCX24B;
	else if (check2)
		format=Epcxformat::PCX8B;
	else 
		return NULL;
	add=(int)(4*(((float)1)-(((float)maxx/(float)4)-((int)((float)maxx/(float)4)))));
	if (add==4)
		add=0;
	if (format==Epcxformat::PCX8B)
	{
		it = pcxs-256*3;
		for (int i=0;i<256;i++)
		{
			pal[i].R=pcx[it++];
			pal[i].G=pcx[it++];
			pal[i].B=pcx[it++];
			pal[i].F='\0';
		}
	}
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int rmask = 0xff000000;
    int gmask = 0x00ff0000;
    int bmask = 0x0000ff00;
    int amask = 0x000000ff;
#else
    int rmask = 0x000000ff;
    int gmask = 0x0000ff00;
    int bmask = 0x00ff0000;
    int amask = 0xff000000;
#endif
	ret = SDL_CreateRGBSurface(SDL_SWSURFACE, maxx, maxy, (format==Epcxformat::PCX8B ? 8 : 24), rmask, gmask, bmask, amask);
	if (format==Epcxformat::PCX8B)
	{
		for(int i=0; i<256; ++i)
		{
			SDL_Color pr;
			pr.r = pal[i].R;
			pr.g = pal[i].G;
			pr.b = pal[i].B;
			pr.unused = pal[i].F;
			(*(ret->format->palette->colors+i)) = pr;
		}
		for(y=maxy; y>0; --y)
		{
			it = 0xc + (y-1)*maxx;
			for(int j=0; j<maxx; ++j)
				*((char*)ret->pixels + ret->format->BytesPerPixel * (y*(maxx+add) + j)) = pcx[it+j];
			for(int j=0; j<add; ++j)
				*((char*)ret->pixels + ret->format->BytesPerPixel * (y*(maxx+add) + maxx + j)) = 0;
		}
	}
	else
	{
		for(int y=maxy; y>0; --y)
		{
			it = 0xc + (y-1)*maxx*3;
			for(int j=0; j<maxx*3; ++j)
			{
				*((char*)ret->pixels + (y*(maxx+add) + j)) = pcx[it+j];
			}
			for(int j=0; j<add*3; ++j)
			{
				*((char*)ret->pixels + (y*(maxx+add) + j)) = 0;
			}
		}
	}
	return ret;
}
SDL_Surface * CLodHandler::loadBitmap(std::string fname)
{
	unsigned char * pcx;
	std::transform(fname.begin(),fname.end(),fname.begin(),toupper);
	fname.replace(fname.find_first_of('.'),fname.find_first_of('.')+4,".PCX");
	int index=-1;
	for (int i=0;i<entries.size();i++)
	{
		std::string buf1 = std::string((char*)entries[i].name);
		//std::transform(buf1.begin(), buf1.end(), buf1.begin(), (int(*)(int))toupper);
		if(buf1==fname)
		{
			index=i;
			break;
		}
	}
	FLOD.seekg(entries[index].offset,std::ios_base::beg);
	if (entries[index].size==0) //file is not compressed
	{
		pcx = new unsigned char[entries[index].realSize];
		FLOD.read((char*)pcx,entries[index].realSize);
	}
	else 
	{
		unsigned char * pcd = new unsigned char[entries[index].size];
		FLOD.read((char*)pcd,entries[index].size);
		infs2(pcd,entries[index].size,entries[index].realSize,pcx);
	}
	CPCXConv cp;
	cp.openPCX((char*)pcx,entries[index].realSize);
	cp.convert();
	cp.saveBMP("vctemp.bmp");
	SDL_Surface * ret = SDL_LoadBMP("vctemp.bmp");
	boost::filesystem::path p("vctemp.bmp");
	boost::filesystem::remove(p);
	//return cp.getSurface();
	return ret;
}

int CLodHandler::decompress (unsigned char * source, int size, int realSize, std::string & dest)
{
	std::ofstream lb;
	lb.open("lodbuf\\buf.gz", std::ios::out|std::ios::binary);
	for(int i=0; i<size; ++i)
	{
		lb<<source[i];
	}
	lb.close();

	FILE * inputf = fopen("lodbuf\\buf.gz", "rb+");
	FILE * outputf = fopen(dest.c_str(), "wb+");

	int ret = infm(inputf, outputf);
	fclose(inputf);
	fclose(outputf);
	return ret;
} 

int CLodHandler::infm(FILE *source, FILE *dest, int wBits)
{
	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[NLoadHandlerHelp::fCHUNK];
	unsigned char out[NLoadHandlerHelp::fCHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, wBits);
	if (ret != Z_OK)
		return ret;

	/* decompress until deflate stream ends or end of file */
	do
	{
		strm.avail_in = fread(in, 1, NLoadHandlerHelp::fCHUNK, source);
		if (ferror(source)) {
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}
		if (strm.avail_in == 0)
			break;
		strm.next_in = in;

		/* run inflate() on input until output buffer not full */
		do
		{
			strm.avail_out = NLoadHandlerHelp::fCHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			//assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret)
			{
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;	 /* and fall through */
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return ret;
			}
			have = NLoadHandlerHelp::fCHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest))
			{
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
CDefHandler * CLodHandler::giveDef(std::string defName) // TODO: zamienic
{
	std::vector<std::string> pom;
	pom.push_back(defName);
	return extractManyFiles(pom)[0];
}
std::vector<CDefHandler *> CLodHandler::extractManyFiles(std::vector<std::string> defNamesIn)
{
	std::vector<CDefHandler *> ret(defNamesIn.size());
	for(int hh=0; hh<defNamesIn.size(); ++hh)
	{
		std::transform(defNamesIn[hh].begin(), defNamesIn[hh].end(), defNamesIn[hh].begin(), (int(*)(int))toupper);
	}
	std::ofstream FOut;
	int i;
	std::vector<char> found(defNamesIn.size(), 0);
	for (int i=0;i<totalFiles;i++)
	{
		//std::cout<<'\r'<<"Reading defs: "<<(100.0*i)/((float)(totalFiles))<<"%      ";
		std::string buf1 = std::string((char*)entries[i].name);
		std::transform(buf1.begin(), buf1.end(), buf1.begin(), (int(*)(int))toupper);
		bool exists = false;
		int curDef;
		for(int hh=0; hh<defNamesIn.size(); ++hh)
		{
			if(buf1==defNamesIn[hh])
			{
				exists = true;
				found[hh] = '\1';
				curDef = hh;
				break;
			}
		}
		if(!exists)
			continue;
		FLOD.seekg(entries[i].offset,std::ios_base::beg);
		//std::string bufff = (lodName.substr(0, lodName.size()-4) + "\\" + (char*)entries[i].name);
		unsigned char * outp;
		if (entries[i].size==0) //file is not compressed
		{
			outp = new unsigned char[entries[i].realSize];
			FLOD.read((char*)outp, entries[i].realSize);
			CDefHandler * nh = new CDefHandler;
			nh->openFromMemory(outp, entries[i].realSize, std::string((char*)entries[i].name));
			ret[curDef] = nh;

		}
		else //we will decompressing file
		{
			outp = new unsigned char[entries[i].size];
			FLOD.read((char*)outp, entries[i].size);
			FLOD.seekg(0, std::ios_base::beg);
			unsigned char * decomp = NULL;
			int decRes = infs2(outp, entries[i].size, entries[i].realSize, decomp);
			CDefHandler * nh = new CDefHandler;
			nh->openFromMemory(decomp, entries[i].realSize, std::string((char*)entries[i].name));
			ret[curDef] = nh;
		}
		delete outp;
	}
	//std::cout<<'\r'<<"Reading defs: 100%    "<<std::endl;
	for(int hh=0; hh<found.size(); ++hh)
	{
		if(!found[hh])
		{
			for(int ff=0; ff<hh; ++ff)
			{
				if(defNamesIn[ff]==defNamesIn[hh])
				{
					ret[hh]=ret[ff];
				}
			}
		}
	}
	//std::cout<<"*** Archive: "+FName+" closed\n";
	return ret;
}
int CLodHandler::infs(unsigned char * in, int size, int realSize, std::ofstream & out, int wBits)
{
	int ret;
	unsigned have;
	z_stream strm;
	unsigned char inx[NLoadHandlerHelp::fCHUNK];
	unsigned char outx[NLoadHandlerHelp::fCHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, wBits);
	if (ret != Z_OK)
		return ret;
	int chunkNumber = 0;
	do
	{
		int readBytes = 0;
		for(int i=0; i<NLoadHandlerHelp::fCHUNK && (chunkNumber * NLoadHandlerHelp::fCHUNK + i)<size; ++i)
		{
			inx[i] = in[chunkNumber * NLoadHandlerHelp::fCHUNK + i];
			++readBytes;
		}
		++chunkNumber;
		strm.avail_in = readBytes;
		//strm.avail_in = fread(inx, 1, NLoadHandlerHelp::fCHUNK, source);
		/*if (in.bad())
		{
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}*/
		if (strm.avail_in == 0)
			break;
		strm.next_in = inx;

		/* run inflate() on input until output buffer not full */
		do
		{
			strm.avail_out = NLoadHandlerHelp::fCHUNK;
			strm.next_out = outx;
			ret = inflate(&strm, Z_NO_FLUSH);
			//assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret)
			{
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;	 /* and fall through */
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return ret;
			}
			have = NLoadHandlerHelp::fCHUNK - strm.avail_out;
			/*if (fwrite(out, 1, have, dest) != have || ferror(dest))
			{
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}*/
			out.write((char*)outx, have);
			if(out.bad())
			{
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

int CLodHandler::infs2(unsigned char * in, int size, int realSize, unsigned char *& out, int wBits)
{
	int ret;
	unsigned have;
	z_stream strm;
	unsigned char inx[NLoadHandlerHelp::fCHUNK];
	unsigned char outx[NLoadHandlerHelp::fCHUNK];
	out = new unsigned char [realSize];
	int latPosOut = 0;

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, wBits);
	if (ret != Z_OK)
		return ret;
	int chunkNumber = 0;
	do
	{
		int readBytes = 0;
		for(int i=0; i<NLoadHandlerHelp::fCHUNK && (chunkNumber * NLoadHandlerHelp::fCHUNK + i)<size; ++i)
		{
			inx[i] = in[chunkNumber * NLoadHandlerHelp::fCHUNK + i];
			++readBytes;
		}
		++chunkNumber;
		strm.avail_in = readBytes;
		//strm.avail_in = fread(inx, 1, NLoadHandlerHelp::fCHUNK, source);
		/*if (in.bad())
		{
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}*/
		if (strm.avail_in == 0)
			break;
		strm.next_in = inx;

		/* run inflate() on input until output buffer not full */
		do
		{
			strm.avail_out = NLoadHandlerHelp::fCHUNK;
			strm.next_out = outx;
			ret = inflate(&strm, Z_NO_FLUSH);
			//assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret)
			{
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;	 /* and fall through */
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return ret;
			}
			have = NLoadHandlerHelp::fCHUNK - strm.avail_out;
			/*if (fwrite(out, 1, have, dest) != have || ferror(dest))
			{
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}*/
			//out.write((char*)outx, have);
			for(int oo=0; oo<have; ++oo)
			{
				out[latPosOut] = outx[oo];
				++latPosOut;
			}
			/*if(out.bad())
			{
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}*/
		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void CLodHandler::extract(std::string FName)
{
	
	std::ofstream FOut;
	int i;

	//std::cout<<" done\n";
	for (int i=0;i<totalFiles;i++)
	{
		FLOD.seekg(entries[i].offset,std::ios_base::beg);
		std::string bufff = (FName.substr(0, FName.size()-4) + "\\" + (char*)entries[i].name);
		unsigned char * outp;
		if (entries[i].size==0) //file is not compressed
		{
			outp = new unsigned char[entries[i].realSize];
			FLOD.read((char*)outp, entries[i].realSize);
			std::ofstream out;
			out.open(bufff.c_str(), std::ios::binary);
			if(!out.is_open())
			{
				std::cout<<"Unable to create "<<bufff;
			}
			else
			{
				for(int hh=0; hh<entries[i].realSize; ++hh)
				{
					out<<*(outp+hh);
				}
				out.close();
			}
		}
		else 
		{
			outp = new unsigned char[entries[i].size];
			FLOD.read((char*)outp, entries[i].size);
			FLOD.seekg(0, std::ios_base::beg);
			std::ofstream destin;
			destin.open(bufff.c_str(), std::ios::binary);
			//int decRes = decompress(outp, entries[i].size, entries[i].realSize, bufff);
			int decRes = infs(outp, entries[i].size, entries[i].realSize, destin);
			destin.close();
			if(decRes!=0)
			{
				std::cout<<"LOD Extraction error"<<"  "<<decRes<<" while extracting to "<<bufff<<std::endl;
			}
		}
		//for (int j=0; j<entries[i].size; j++)
		//	FOut << outp[j];
		//FOut.flush();
		delete outp;
		//FOut.close();
		//std::cout<<"*** done\n";
	}
	FLOD.close();
	//std::cout<<"*** Archive: "+FName+" closed\n";
}

void CLodHandler::extractFile(std::string FName, std::string name)
{
	//std::ifstream FLOD;
	std::ofstream FOut;
	int i;

	//std::string Ts;
	////std::cout<<"*** Loading FAT ... \n";
	//FLOD.open(FName.c_str(),std::ios::binary);
	////std::cout<<"*** Archive: "+FName+" loaded\n";
	//FLOD.seekg(8,std::ios_base::beg);
	//unsigned char temp[4];
	//FLOD.read((char*)temp,4);
	//totalFiles = readNormalNr(temp,4);
	//FLOD.seekg(0x5c,std::ios_base::beg);
	//entries.reserve(totalFiles);
	////std::cout<<"*** Loading FAT ...\n";
	//for (int i=0; i<totalFiles; i++)
	//{
	//	entries.push_back(Entry());
	//	//FLOD.read((char*)entries[i].name,12);
	//	char * bufc = new char;
	//	bool appending = true;
	//	for(int kk=0; kk<12; ++kk)
	//	{
	//		FLOD.read(bufc, 1);
	//		if(appending)
	//		{
	//			entries[i].name[kk] = *bufc;
	//		}
	//		else
	//		{
	//			entries[i].name[kk] = 0;
	//			appending = false;
	//		}
	//	}
	//	delete bufc;
	//	FLOD.read((char*)entries[i].hlam_1,4);
	//	FLOD.read((char*)temp,4);
	//	entries[i].offset=readNormalNr(temp,4);
	//	FLOD.read((char*)temp,4);
	//	entries[i].realSize=readNormalNr(temp,4);
	//	FLOD.read((char*)entries[i].hlam_2,4);
	//	FLOD.read((char*)temp,4);
	//	entries[i].size=readNormalNr(temp,4);
	//}
	//std::cout<<" done\n";
	std::transform(name.begin(), name.end(), name.begin(), (int(*)(int))toupper);
	for (int i=0;i<totalFiles;i++)
	{
		std::string buf1 = std::string((char*)entries[i].name);
		std::transform(buf1.begin(), buf1.end(), buf1.begin(), (int(*)(int))toupper);
		if(buf1!=name)
			continue;
		FLOD.seekg(entries[i].offset,std::ios_base::beg);
		std::string bufff = (FName.substr(0, FName.size()-4) + "\\" + (char*)entries[i].name);
		unsigned char * outp;
		if (entries[i].size==0) //file is not compressed
		{
			outp = new unsigned char[entries[i].realSize];
			FLOD.read((char*)outp, entries[i].realSize);
			std::ofstream out;
			out.open(bufff.c_str(), std::ios::binary);
			if(!out.is_open())
			{
				std::cout<<"Unable to create "<<bufff;
			}
			else
			{
				for(int hh=0; hh<entries[i].realSize; ++hh)
				{
					out<<*(outp+hh);
				}
				out.close();
			}
		}
		else //we will decompressing file
		{
			outp = new unsigned char[entries[i].size];
			FLOD.read((char*)outp, entries[i].size);
			FLOD.seekg(0, std::ios_base::beg);
			std::ofstream destin;
			destin.open(bufff.c_str(), std::ios::binary);
			//int decRes = decompress(outp, entries[i].size, entries[i].realSize, bufff);
			int decRes = infs(outp, entries[i].size, entries[i].realSize, destin);
			destin.close();
			if(decRes!=0)
			{
				std::cout<<"LOD Extraction error"<<"  "<<decRes<<" while extracting to "<<bufff<<std::endl;
			}
		}
		//for (int j=0; j<entries[i].size; j++)
		//	FOut << outp[j];
		//FOut.flush();
		delete outp;
		//FOut.close();
		//std::cout<<"*** done\n";
	}
	FLOD.close();
	//std::cout<<"*** Archive: "+FName+" closed\n";
}

int CLodHandler::readNormalNr (unsigned char* bufor, int bytCon, bool cyclic)
{
	int ret=0;
	int amp=1;
	for (int i=0; i<bytCon; i++)
	{
		ret+=bufor[i]*amp;
		amp*=256;
	}
	if(cyclic && bytCon<4 && ret>=amp/2)
	{
		ret = ret-amp;
	}
	return ret;
}

void CLodHandler::init(std::string lodFile)
{
	FLOD;
	std::string Ts;
	//std::cout<<"*** Loading FAT ... \n";
	FLOD.open(lodFile.c_str(),std::ios::binary);
	//std::cout<<"*** Archive: "+FName+" loaded\n";
	FLOD.seekg(8,std::ios_base::beg);
	unsigned char temp[4];
	FLOD.read((char*)temp,4);
	totalFiles = readNormalNr(temp,4);
	FLOD.seekg(0x5c,std::ios_base::beg);
	entries.reserve(totalFiles);
	//std::cout<<"*** Loading FAT ...\n";
	for (int i=0; i<totalFiles; i++)
	{
		entries.push_back(Entry());
		//FLOD.read((char*)entries[i].name,12);
		char * bufc = new char;
		bool appending = true;
		for(int kk=0; kk<12; ++kk)
		{
			FLOD.read(bufc, 1);
			if(appending)
			{
				entries[i].name[kk] = toupper(*bufc);
			}
			else
			{
				entries[i].name[kk] = 0;
				appending = false;
			}
		}
		delete bufc;
		FLOD.read((char*)entries[i].hlam_1,4);
		FLOD.read((char*)temp,4);
		entries[i].offset=readNormalNr(temp,4);
		FLOD.read((char*)temp,4);
		entries[i].realSize=readNormalNr(temp,4);
		FLOD.read((char*)entries[i].hlam_2,4);
		FLOD.read((char*)temp,4);
		entries[i].size=readNormalNr(temp,4);
	}
}
