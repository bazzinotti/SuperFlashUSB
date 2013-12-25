#ifndef UTILITY_H
#define UTILITY_H

#include <QtWidgets>

extern QString RomRamSizeByteLUT[];
extern QMap<int,QString> CartTypeMap;


/* Snes9x Hi/LoROM autodetect code */

int AllASCII(unsigned char *b, int size);

int ScoreHiROM(unsigned char *data);
int ScoreLoROM(unsigned char *data);

int ScoreRomHeader(unsigned char *data);



bool is_headered(QFile &file);

bool isHirom(uchar *data);
bool isHirom2(uchar *loromdata, uchar *hiromdata);

int OpenFiles();
int OpenForReadBin(const char *filename);
int OpenForWriteBin(const char *filename);

//extern uchar MajorCommand,MinorCommand;



class ROM_t
{
public:
    ROM_t()
    {
        offset=0;
        hirom = false;
        headered=false;
        for (uchar i=0; i < 22; i++)
        {
            RomTitle[i]=0;
        }
        RomSizeByte=0;
        SramSizeByte=0;
        file = NULL;
        filename = "";
        finalString="";
        num=1;
    }

    ~ROM_t()
    {
        if (file)
        {
            file->close();
            delete file;
        }
    }

    int open()
    {
        if (file)
        {
            file->close();
            delete file;
        }

        file = new QFile(filename);

        if (!file->open(QIODevice::ReadWrite))
        {
            QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Could not open file"));
            return -1;
        }

        return 1;
    }

    bool isHeadered()
    {
        return headered;
    }

    int setup()
    {
        offset = 0;
        // How to find which item was selected?

        // Autodetect the HiROM/LoROM state
        //return;

        if ((headered = is_headered(*file)))
        {
            //QMessageBox::warning(this, tr("Error"), file->fileName() + tr(" is headered"));
            //file.unmap(data);
            offset = 0x200;
        }

        data = file->map(offset, file->size()-offset);
        hirom = isHirom(data);
        // stored in hirom
        if (hirom)
            offset = 0x8000;
        else
            offset = 0;



        for (uchar i=0; i < 21; i++)
        {
            RomTitle[i] = data[0x7fc0+offset+i];
        }
        //RomTitle[21] = 0;
        // ROM Title All set

        CartTypeByte = data[0x7fd6+offset];

        // Get ROM Size
        RomSizeByte = data[0x7fd7+offset];


        // Get SRAM Size
        SramSizeByte = data[0x7fd8+offset];
        // Headered

        if (RomSizeByte > 14 || SramSizeByte > 14 || RomSizeByte == 0)
        {
            QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Something's Wrong. Did you load the correct file?"));
            //qApp->quit();
            return -1;
        }

        // Check for Correlation between RomSizeByte and actual RomFileSize
        // map RomSizeByte to actual number of bytes
        qint64 targetbytesize = 2048<<(RomSizeByte-1);
        qint64 romfilesize = file->size();
        if (headered)
            romfilesize -= 0x200;
        if (targetbytesize != romfilesize)
        {
            //QMessageBox::question()zzzz
            if (QMessageBox::question(NULL, QObject::tr("Hm..."), QObject::tr("This file is not the same size as the ROM header says it should be\n\nLoad it anyways?")) == QMessageBox::No)
            {
                return -1;
            }
            //QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("The file is not the same size as the ROM header says it should be"));
            //qApp->quit();

        }


        return 0;
    }

    void setString()
    {
        finalString = QString("<b>")+QString("%1").arg(num)+QString(") </b>")+QString(RomTitle)+
                QString("<p>Cart Type: ")+QString(CartTypeMap[CartTypeByte])+
                QString("<p>&nbsp;&nbsp;&nbsp;&nbsp;")+QString("<b>ROM</b>: ")+QString(RomRamSizeByteLUT[RomSizeByte])+QString("<p>&nbsp;&nbsp;&nbsp;&nbsp;")+QString("<b>SRAM</b>: ")+QString(RomRamSizeByteLUT[SramSizeByte]);
    }

    bool isValid()
    {
        if (!(RomSizeByte > 14 || SramSizeByte > 14 || RomSizeByte == 0))
            return true;
        else return false;
    }

    bool hirom;
    uchar *data;
    ushort offset;
    bool headered;
    char RomTitle[21+1];
    u_int8_t RomSizeByte;
    u_int8_t SramSizeByte;
    u_int8_t CartTypeByte;
    QFile *file;
    QString filename;

public:
    quint8 num;
    QString finalString;
};



int dothedo(ROM_t &rom);

class SRAM
{
public:
    SRAM()
    {

        file = NULL;
        filename = "";
        finalString="";

    }

    ~SRAM()
    {
        if (file)
        {
            file->close();
            delete file;
        }
    }




    int setup()
    {
        if (!filename.isEmpty())
        {
            if (file)
            {
                file->close();
                delete file;
            }
            file = new QFile(filename);
            if (!file->open(QIODevice::ReadOnly))
            {
                QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Could not open file"));
                return -1;
            }


            if (file->size() > 0x20000)
            {
                QMessageBox::warning(NULL, QObject::tr("Error"), file->fileName() + QObject::tr("\nFile is too big"));
                return -1;
            }


            // This might only work for Linux/Unix based OS
            finalString = filename.mid(filename.lastIndexOf('/')+1);

            data = file->map(0, file->size());
        }


        return 0;
    }

    QFile *file;
    QString filename;
    QString finalString;
    uchar *data;
};

#endif // UTILITY_H
