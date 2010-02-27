#ifndef	__FDD_H__
#define	__FDD_H__

#pragma once

//*****************************************************************************
//	eUdi
//-----------------------------------------------------------------------------
class eUdi
{
public:
	eUdi(int cyls, int sides);
	~eUdi() { SAFE_DELETE(raw); }
	void Update(); //on raw changed
	int Cyls() const	{ return cyls; }
	int Sides() const	{ return sides; }

	enum { MAX_CYL = 86, MAX_SIDE = 2, MAX_SEC = 32 };
	struct eTrack
	{
		eTrack() : data_len(0), data(NULL), id(NULL), sectors_amount(0) {}
		bool Marker(int pos) const;
		void Write(int pos, byte v, bool marker = false);

		int		data_len;
		byte*	data;
		byte*	id;

		struct eSector
		{
			eSector() : id(NULL), data(NULL) {}
			enum eId { ID_CYL = 0, ID_SIDE, ID_SEC, ID_LEN, ID_AMOUNT };
			int Cyl() const		{ return id[ID_CYL]; }
			int Side() const	{ return id[ID_SIDE]; }
			int Sec() const		{ return id[ID_SEC]; }
			int Len() const		{ return 128 << (id[ID_LEN] & 3); }
			byte*	id;
			byte*	data;
		};
		eSector	sectors[MAX_SEC];
		int		sectors_amount;
	};
	eTrack& Track(int cyl, int side) { return tracks[cyl][side]; }

protected:
	void 	UpdateTrack(int cyl, int side);

protected:
	int		cyls;
	int		sides;
	eTrack	tracks[MAX_CYL][MAX_SIDE];
	byte*	raw;
};

//*****************************************************************************
//	eFdd
//-----------------------------------------------------------------------------
class eFdd
{
public:
	eFdd();
	~eFdd() { SAFE_DELETE(disk); }
	bool DiskPresent() const	{ return disk != NULL; }
	bool WriteProtect() const	{ return write_protect; }
	bool Open(const char* image);
	void Seek(int _cyl, int _side);
	int TSByte() const { return ts_byte; }
	eUdi::eTrack& Track() { return disk->Track(cyl, side); }
	eUdi::eTrack::eSector& Sector(int sec) { return Track().sectors[sec]; }
	void Write(int pos, byte v, bool marker = false) { Track().Write(pos, v, marker); }
	word Crc(byte* src, int size) const;

protected:
	eUdi::eTrack::eSector* GetSector(int cyl, int side, int sec);
	bool WriteSector(int cyl, int side, int sec, byte* data);
	void WriteBlock(int& pos, byte v, int amount, bool marker = false)
	{
		for(int i = 0; i < amount; ++i)
		{
			Track().Write(pos++, v, marker);
		}
	}
	void Format();
	void FormatTrd();
	void CreateTrd();
	bool AddFile(byte* hdr, byte* data);
	bool ReadScl(byte* snbuf);
	bool ReadTrd(byte* snbuf);

	enum { TRD_SIZE = 655360 };
public:
	qword	motor;	// 0 - not spinning, >0 - time when it'll stop
	int		cyl;
	int		optype;	// bits: 0-not modified, 1-write sector, 2-format track

protected:
	int		side;
	int		ts_byte; // cpu.t per byte
	bool	write_protect;
	eUdi*	disk;
};

#endif//__FDD_H__