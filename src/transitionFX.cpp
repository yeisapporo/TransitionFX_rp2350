#include <stdint.h>
#include <Arduino.h>

#define LGFX_AUTODETECT

#include "LGFX_SpriteFX.hpp"
#include "assets.h" // some example images for this demo

class LGFX : public lgfx::LGFX_Device
{
  private:
    const int pinLED = 27;
    const int pinCS = 17;
    const int pinReset = 22;
    const int pinDC = 28;
    const int pinMOSI = 19;
    const int pinSck = 18;
    const int pinMISO = -1;
    const int pinBusy = -1;

    lgfx::Panel_ILI9341 panel;
    lgfx::Bus_SPI bus;

    void initBusConfig()
    {
        auto cfg = this->bus.config();
        cfg.spi_host = 0;
        cfg.spi_mode = 0;
        cfg.freq_write = 70000000;
        cfg.freq_read = 16000000;
        cfg.pin_sclk = this->pinSck;
        cfg.pin_mosi = this->pinMOSI;
        cfg.pin_miso = this->pinMISO;
        cfg.pin_dc = this->pinDC;
        this->bus.config(cfg);
    }

    void initPanelConfig()
    {
        auto cfg = this->panel.config();
        cfg.pin_cs = this->pinCS;
        cfg.pin_rst = this->pinReset;
        cfg.pin_busy = -1;
        cfg.panel_width = 240;
        cfg.panel_height = 320;
        cfg.offset_x = 0;
        cfg.offset_y = 0;
        cfg.offset_rotation = 2;
        cfg.dummy_read_pixel = 8;
        cfg.dummy_read_bits = 1;
        cfg.readable = false;
        cfg.invert = false;
        cfg.rgb_order = false;
        cfg.dlen_16bit = false;
        cfg.bus_shared = false;
        this->panel.config(cfg);
    }

  public:
    LGFX(void)
    {
        this->initBusConfig();
        this->initPanelConfig();
        this->panel.setBus(&this->bus);
        setPanel(&this->panel);
        pinMode(this->pinLED, OUTPUT);
    }

    void led(bool on)
    {
        digitalWrite(this->pinLED, on ? HIGH : LOW);
    }
};


struct easingFuncDesc_t
{
  const char* name;
  lgfx::easing::easing_fn_t func;
};


struct img_t
{
  const unsigned char* data;
  const unsigned int len;
  const uint32_t w;
  const uint32_t h;
};


static LGFX tft;
static LGFX_SpriteFx *sptr;

const img_t dog      = img_t { dog_200_200_jpg,       dog_200_200_jpg_len,       200, 200 };
const img_t logo     = img_t { lgfx_logo_201x197_jpg, lgfx_logo_201x197_jpg_len, 201, 197 };
const img_t palette1 = img_t { palette_300x128_jpg,   palette_300x128_jpg_len,   300, 128 };
const img_t palette2 = img_t { palette_128x200_jpg,   palette_128x200_jpg_len,   128, 200 };


const img_t *images[] = { &palette1, &palette2, &dog, &logo };

const easingFuncDesc_t easingFunctions[] =
{
  { "easeOutBounce",    lgfx::easing::easeOutBounce    },
  { "easeOutElastic",   lgfx::easing::easeOutElastic   },
  // { "easeInElastic",    lgfx::easing::easeInElastic    },
  // { "easeInOutElastic", lgfx::easing::easeInOutElastic },
  // { "easeInQuad",       lgfx::easing::easeInQuad       },
  // { "easeOutQuad",      lgfx::easing::easeOutQuad      },
  // { "easeInOutQuad",    lgfx::easing::easeInOutQuad    },
  // { "easeInCubic",      lgfx::easing::easeInCubic      },
  // { "easeOutCubic",     lgfx::easing::easeOutCubic     },
  // { "easeInOutCubic",   lgfx::easing::easeInOutCubic   },
  // { "easeInQuart",      lgfx::easing::easeInQuart      },
  // { "easeOutQuart",     lgfx::easing::easeOutQuart     },
  // { "easeInOutQuart",   lgfx::easing::easeInOutQuart   },
  // { "easeInQuint",      lgfx::easing::easeInQuint      },
  // { "easeOutQuint",     lgfx::easing::easeOutQuint     },
  // { "easeInOutQuint",   lgfx::easing::easeInOutQuint   },
  // { "easeInSine",       lgfx::easing::easeInSine       },
  // { "easeOutSine",      lgfx::easing::easeOutSine      },
  // { "easeInOutSine",    lgfx::easing::easeInOutSine    },
  // { "easeInExpo",       lgfx::easing::easeInExpo       },
  // { "easeOutExpo",      lgfx::easing::easeOutExpo      },
  // { "easeInOutExpo",    lgfx::easing::easeInOutExpo    },
  // { "easeInCirc",       lgfx::easing::easeInCirc       },
  // { "easeOutCirc",      lgfx::easing::easeOutCirc      },
  // { "easeInOutCirc",    lgfx::easing::easeInOutCirc    },
  // { "easeInBack",       lgfx::easing::easeInBack       },
  // { "easeOutBack",      lgfx::easing::easeOutBack      },
  // { "easeInOutBack",    lgfx::easing::easeInOutBack    },
  // { "linear",           lgfx::easing::linear           },
};


void clearZone( int x, int y, int w, int h, uint32_t delay_ms )
{
  delay( delay_ms );
  tft.fillRect( x, y, w, h, TFT_WHITE );
}



void setup()
{
  Serial.begin( 115200 );
  pinMode(25, OUTPUT);
  digitalWrite(25, HIGH);
  tft.init();
  tft.led(true);
  tft.startWrite();

  tft.setTextDatum( TL_DATUM );
  tft.setTextPadding( tft.width() );
  tft.setTextColor( TFT_WHITE, TFT_BLACK );
  tft.setTextSize(2);
  tft.setTextWrap( false );

  sptr = new LGFX_SpriteFx(&tft);
  sptr->setPsram( false );
}


void loop()
{
  static uint32_t imgidx = 0;
  static uint32_t imgcount = sizeof(images)/sizeof(img_t*);
  static uint32_t min_delay_ms = 8;

  auto imgmod = imgidx%imgcount;
  min_delay_ms = (imgmod==0) ? 8-min_delay_ms : min_delay_ms;

  auto img  = images[imgmod];
  auto posx = tft.width()/2  - img->w/2;
  auto posy = tft.height()/2 - img->h/2;

  if( ! sptr->createSprite( img->w, img->h ) ) return;

  // highlight zone
  tft.fillRect( posx,   posy,   sptr->width(),   sptr->height(),   TFT_WHITE );
  tft.drawRect( posx-1, posy-1, sptr->width()+2, sptr->height()+2, TFT_RED );

  // fill sprite with image
  sptr->drawJpg( img->data, img->len, 0, 0, img->w, img->h );

  sptr->stretchRTL( posx, posy, min_delay_ms );  // clearZone( posx, posy, img->w, img->h, 500 );
  sptr->stretchBTT( posx, posy, min_delay_ms );  // clearZone( posx, posy, img->w, img->h, 500 );
  sptr->stretchLTR( posx, posy, min_delay_ms );  // clearZone( posx, posy, img->w, img->h, 500 );
  sptr->stretchTTB( posx, posy, min_delay_ms );  // clearZone( posx, posy, img->w, img->h, 500 );

  size_t easing_functions_count = sizeof(easingFunctions)/sizeof(easingFuncDesc_t);

  for( int i=0;i<easing_functions_count;i++ ) {

    auto easingFunc = easingFunctions[i].func;
    auto funcName   = easingFunctions[i].name;

    Serial.printf("Easing: %s\n", funcName );
    tft.setCursor( 0, 0 );
    tft.printf("Easing: %-32s", funcName );


    sptr->spinC(  posx, posy, 1000, easingFunc );       // clearZone( posx, posy, img->w, img->h, 500 );
    sptr->spinA(  posx, posy, 1000, easingFunc );       // clearZone( posx, posy, img->w, img->h, 500 );

    sptr->sliceH(  posx, posy, 0, 1000, easingFunc );   // clearZone( posx, posy, img->w, img->h, 500 );
    sptr->sliceV(  posx, posy, 0, 1000, easingFunc );   // clearZone( posx, posy, img->w, img->h, 500 );

    sptr->sliceH(  posx, posy, 4, 1000, easingFunc );   // clearZone( posx, posy, img->w, img->h, 500 );
    sptr->sliceV(  posx, posy, 4, 1000, easingFunc );   // clearZone( posx, posy, img->w, img->h, 500 );

    sptr->sliceH(  posx, posy, 8, 1000, easingFunc );   // clearZone( posx, posy, img->w, img->h, 500 );
    sptr->sliceV(  posx, posy, 8, 1000, easingFunc );   // clearZone( posx, posy, img->w, img->h, 500 );

    sptr->sliceH(  posx, posy, 16, 1000, easingFunc );  // clearZone( posx, posy, img->w, img->h, 500 );
    sptr->sliceV(  posx, posy, 16, 1000, easingFunc );  // clearZone( posx, posy, img->w, img->h, 500 );

    sptr->sliceH(  posx, posy, 255, 1000, easingFunc ); // clearZone( posx, posy, img->w, img->h, 500 );
    sptr->sliceV(  posx, posy, 255, 1000, easingFunc ); // clearZone( posx, posy, img->w, img->h, 500 );

  }

  tft.setCursor( 0, 0 );
  tft.printf("%64s", "" );

  // clear zone
  tft.fillRect( posx-1, posy-1, img->w+2, img->h+2, TFT_BLACK );

  imgidx++;
}
