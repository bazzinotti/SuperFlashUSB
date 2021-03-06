#include "write.h"


void Write(QProgressBar *pb, QFile *f)
{
    // YUMMY
    file = f;
    progressBar = pb;
    InitWrite();
    
    
    
    // Read Data From SUPERFLASH
    WriteDataFromFile();
   
}

void LoadWriteBuffer()
{
    data[0] = aal;
    data[1] = aah;
    data[2] = aab;

    /*data[3] = l;
    data[4] = h;
    data[5] = b;
    
	data[6] = chunks&0xff;
	data[7] = (chunks&0xff00)>>8;
	data[8] = (chunks&0xff0000)>>16;
    
	data[9] = leftover_bytes&0xff;
	data[10] = (leftover_bytes&0xff00)>>8;
	data[11] = (leftover_bytes&0xff0000)>>16;*/
}

void InitWrite()
{
    chunks = numbytes / DERP_SIZE;
    leftover_bytes = numbytes % DERP_SIZE; 
    
    LoadWriteBuffer();
    
    uint8_t bmrt = LIBUSB_RECIPIENT_DEVICE|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_ENDPOINT_OUT;
    cout << "Write: bmrt = ";
    printf ( "%x\n", bmrt);
    redo:
	r = libusb_control_transfer(dev_handle, bmrt,WRITE, ADDR, rom_or_sram, &data[0], WRITE_PACKET_SIZE, 500);
  	if(r == WRITE_PACKET_SIZE ) //we wrote successfully
    {
#ifdef DEBUG
         cout<<"Writing control transfer Successful!"<<endl;
#endif
    }
  	else
    {
  		cout<<"Write Error " << r <<endl;
        goto redo;
    }
}


void WriteDataFromFile()
{
    int i=0; //storechunks=chunks;
    if (chunks != 0)
    {
        for (; chunks > 0; chunks--)
        {  
            // Get a 32K section from USB
            file->read((char*)&data[0], DERP_SIZE);
            redo:
            r = libusb_control_transfer(dev_handle, LIBUSB_RECIPIENT_DEVICE|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_ENDPOINT_OUT,WRITE, DATA, (uint16_t) DERP_SIZE, &data[0], DERP_SIZE, 50);
            if(r == DERP_SIZE ) //we wrote the 4 bytes successfully
            {
             // Write it to file
                //cout << "Erase Cmd Sent, Verifying: \n";
        
                while (VerifyWrite() != 0);
                progressBar->setValue(++i*DERP_SIZE);
              //cout<<"Read in 64 Bytes"<<endl;
                //loadBar(i++*DERP_SIZE, (DERP_SIZE*storechunks)+leftover_bytes, ((DERP_SIZE*storechunks)+leftover_bytes)/2, 50);
            }
            else if (r == LIBUSB_ERROR_PIPE || r == LIBUSB_ERROR_OTHER)
            {
                cout << "\tWrite: PIPE/Other Error: " << r << endl;
                printf( "%d\n", errno );
                //ResetAddress();
                goto redo;
            }
            else
            {
    	        cout<<"\tWrite Error: " << r <<endl;
                printf( "%d\n", errno );
                goto redo;
            }
        }
    }

    if (leftover_bytes != 0)
    {
        fread(&data[0], 1, leftover_bytes, fh);
        redo2:
        r = libusb_control_transfer(dev_handle, LIBUSB_RECIPIENT_DEVICE|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_ENDPOINT_IN,READ, leftover_bytes, 0x0000, &data[0], leftover_bytes, 50);
        if(r == (int)leftover_bytes ) //we wrote the 4 bytes successfully
        {
            while (VerifyWrite() != 0);
            
            //loadBar(i++*DERP_SIZE,(DERP_SIZE*storechunks)+leftover_bytes, ((DERP_SIZE*storechunks)+leftover_bytes)/2, 50);
          //cout<<"Read in " << leftover_bytes <<" Bytes"<<endl;
        }
        else if (r == LIBUSB_ERROR_PIPE || r == LIBUSB_ERROR_OTHER)
        {
            cout << "\tWrite: PIPE/Other Error: " << r << endl;
            printf( "%d\n", errno );
            //ResetAddress();
            goto redo2;
        }
        else
        {
	        cout<<"\tWrite Error: " << r <<endl;
            printf( "%d\n", errno );
            goto redo2;
        }
    }
    
    SetLEDWithByte(0);
}

int VerifyWrite()
{
    redo:
    r = libusb_control_transfer(dev_handle, LIBUSB_RECIPIENT_DEVICE|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_ENDPOINT_IN,WRITE, 0, 0x0000, &data[0], 1, 500);
    if(r == 1 ) 
    {
        //cout << "Flash Status Byte: ";
        //printf ("%x\n", data[0]);
        
        if (rom_or_sram == ROM)
        {
            if (data[0] == 0x80)
            {
                //cout << "Complete!\n";
                return 0;
            }
            else
            {
                return 1;
            }
        }
        else if (rom_or_sram == SRAM)
        {
            if (data[0] == 0)
            {
                return 0;
            }
            else
            {
                return 1;
            }
        }
    }
    else if (r == LIBUSB_ERROR_PIPE || r == LIBUSB_ERROR_OTHER)
    {
        cout << "\tWrite: PIPE/Other Error: " << r << endl;
        printf( "%d\n", errno );
        //ResetAddress();
        goto redo;
    }
    else
    {
        cout<<"\tWrite: Error: " << r <<endl;
        printf( "%d\n", errno );
        goto redo;
    }
}

