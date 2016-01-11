#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <Windows.h>
using namespace std;

struct FLOAT2
{
	float val[2];
};

struct FLOAT3
{
	float val[3];
};

struct INT3
{
	int val[3];
};

ostream& operator<<(ostream& os, const FLOAT3 & v)
{
	os << v.val[0] << ", " << v.val[1] << ", " << v.val[2];
	return os;
}

ostream& operator<<(ostream& os, const INT3 & v)
{
	os << v.val[0] << "/" << v.val[1] << "/" << v.val[2];
	return os;
}

ostream& operator<<(ostream& os, const FLOAT2 & vt)
{
	os << vt.val[0] << ", " << vt.val[1];
	return os;
}

size_t FindNextLine(FILE * file)
{
	size_t sz = 0;
	char p = '0';

	cout << "/* ";
	while (p != '\n')
	{
		sz += fread(&p, sizeof(p), 1, file);
		cout << p;
	}
	cout << " */" << endl;
	return sz;
}

size_t FindVertex(FILE * file, char * info)
{
	char p = '1';
	size_t sz = 0;

	while (true) {
		if (p != 'o')
		{
			while (p != '\n')
			{
				sz += fread(&p, sizeof(p), 1, file);
				cout << p;
			}
			cout << endl;
		}
		else // p == 'o'
		{
			while (p != '\n')
			{
				sz += fread(&p, sizeof(p), 1, file);
				cout << p;
			}
			cout << endl;
		}
		sz += fread(&p, sizeof(p), 1, file);

		if (p == 'v')
		{
			*info = p;
			return sz;
		}
	}
}

INT3 ChangeForFace(FILE * file, char * err)
{
	char value[10];

	int rindex = 0, index = 0;
	char p;
	INT3 result{ 0, 0, 0 };

	while (true)
	{
		::fread(&p, sizeof(char), 1, file);
		if (p == ' ' || p == '\n') break;
		else if (p == '/')
		{
			if (index == 0) break;

			value[index] = '\0';
			result.val[rindex++] = atoi(value);
			index = 0;
		}
		else
			value[index++] = p;
	}
	value[index] = '\0';
	result.val[rindex++] = atoi(value);

	*err = p;
	return result;
}

//INT3 ChangeForFace(FILE * file, char * err)
//{
//	char value[10];
//
//	int rindex = 0, index = 0;
//	char p = '1';
//	INT3 result{ 0, 0, 0 };
//	int tmp = -1;
//
//	while (true)
//	{
//		fread(&tmp, sizeof(int), 1, file);
//		result.val[index++] = tmp;
//
//		fread(&p, sizeof(char), 1, file);
//
//		if (p == ' ' || p == '\0' || p == '\n') break;
//	}
//	*err = p;
//	return result;
//}

float ChangeToFloat(FILE * file, char * info)
{
	char value[15];

	int index = 0;
	char p = '1';

	while (true)
	{
		fread(&p, sizeof(char), 1, file);

		if (p == ' ' || p == '\0' || p == '\n')
		{
			*info = p;
			break;
		}
		value[index++] = p;
	}

	value[index] = '\0';
	return atof(value);
}



float ChangeToFloatPoint6(FILE * file)
{
	char p[9];

	fread(&p, sizeof(char), 8, file);
	p[8] = '\0';
	if (p[0] == '-')
	{
		float val = atof(p);
		char p2[2];
		fread(&p2, sizeof(char), 1, file);
		p2[1] = '\0';
		return -val + (0.000001 * atoi(p2));
	}

	return atof(p);
}

FLOAT3 NextFloat3(FILE * file)
{
	FLOAT3 result{ 0,0,0 };
	char info = '-1';

	for (int i = 0; i < 3; ++i)
	{
		result.val[i] = ChangeToFloat(file, &info);
		if (info == '\n')
			break;
	}
	return result;
}

bool WordRead(FILE * file, char out[])
{
	char p = '0';
	long sz = 0;

	while (true)
	{
		fread(&p, sizeof(char), 1, file);
		if (p == ' ' || p == '\0' || p == '\n')
		{
			out[sz] = '\0';

			int index = 0;
			while ((p == ' ' || p == '\0' || p == '\n'))
			{
				fread(&p, sizeof(char), 1, file);
				index++;
			if (feof(file)) 
				return true;
			}
			//cout << "end : " << p << endl;
			fseek(file, -1, SEEK_CUR);
			break;
		}
		if (feof(file))
			return true;

		out[sz++] = p;
	}

	return false;
}

int main()
{
	char p;

	FILE * file;
	file = fopen("sword.obj", "rb");
	size_t sz;

	sz = 0;//fread(&p, sizeof(p), 2, file);
	FLOAT3 v3Temp;
	FLOAT2 v2Temp;
	INT3 i3Temp;

	ZeroMemory(&i3Temp, sizeof(i3Temp));
	ZeroMemory(&v2Temp, sizeof(v2Temp));
	ZeroMemory(&v3Temp, sizeof(v3Temp));

	//FindVertex(file, &p);
	fread(&p, sizeof(p), 1, file); // 공백 제거


	while (file)
	{
		char tmp[20];
		if (WordRead(file, tmp)) break;
		cout << tmp << " : " ;

		if (feof(file))
			break;
		if (!strcmp(tmp, "v"))
		{
			v3Temp = NextFloat3(file);
			cout << v3Temp << endl;
		}
		else if (!strcmp(tmp, "vt"))
		{
			//fread(&p, sizeof(p), 1, file); // 공백 제거
			v3Temp = NextFloat3(file);
			cout << v3Temp << endl;
		}
		else if (!strcmp(tmp, "vn"))
		{
			v3Temp = NextFloat3(file);
			cout << v3Temp << endl;
		}
		else if (!strcmp(tmp, "f"))
		{
			//fread(&p, sizeof(p), 1, file); // 공백 제거
			while (true)
			{
				char tpc;
				cout << ChangeForFace(file, &tpc) << " ";
				if (tpc == '\n')
					break;
			}
			cout << endl;
		}
		else
			FindNextLine(file);
	}
	//	if (p == 'o' || p == '#')
	//		FindNextLine(file);

	//	else if (p == 'u')
	//	{
	//		while (p != '\n')
	//		{
	//			fread(&p, sizeof(p), 1, file); // 공백 제거
	//			cout << p;
	//		}
	//		cout << endl;
	//	}
	//	else if (p == 'v')
	//	{
	//		fread(&p, sizeof(p), 1, file);
	//		if (p == 't')
	//		{
	//			cout << "vt : ";
	//			fread(&p, sizeof(p), 1, file); // 공백 제거

	//			for (int i = 0; i < 2; ++i)
	//				v2Temp.val[i] = ChangeToFloat(file);

	//			cout << v2Temp << endl;

	//		}
	//		else if (p == 'n')
	//		{
	//			cout << "vn : ";
	//			fread(&p, sizeof(p), 1, file); // 공백 제거

	//			for (int i = 0; i < 3; ++i)
	//				v3Temp.val[i] = ChangeToFloat(file);

	//			cout << v3Temp << endl;
	//		}
	//		else if (p == ' ')
	//		{
	//			cout << "v : ";
	//			char fVal[12];
	//			for (int i = 0; i < 3; ++i)
	//				v3Temp.val[i] = ChangeToFloat(file);

	//			cout << v3Temp << endl;

	//		}
	//	}
	//	else if (p == 'f')
	//	{
	//		fread(&p, sizeof(p), 1, file); // 공백 제거
	//		cout << "f : ";

	//		while (true)
	//		{
	//			char tpc;
	//			cout << ChangeForFace(file, &tpc) << " ";
	//			if (tpc == '\n')
	//				break;
	//		}
	//		cout << endl;
	//	}
	//	else if (p == ' ')
	//		void(0);

	//	fread(&p, sizeof(p), 1, file);
	//}
	fclose(file);
}

