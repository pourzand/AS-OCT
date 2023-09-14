#ifndef __misc_h
#define __misc_h

#ifndef __ucla_v5b_h_
#define __ucla_v5b_h_

extern "C" {
#include "ucla_v5b.h" 
}
#endif // !__ucla_v5b_h_

// Change big endian to little endian or vice versa.
inline
void flip32(void* p)
{
  unsigned int tmp = *(unsigned int*)p;
  ((char*)p)[0] = ((char*)&tmp)[3];
  ((char*)p)[1] = ((char*)&tmp)[2];
  ((char*)p)[2] = ((char*)&tmp)[1];
  ((char*)p)[3] = ((char*)&tmp)[0];
};

// Switch a default big endian PACS header (SPARC) to little endian (X86).
inline
void flip_pacs_header_byte_order(PACS_HEADER* p)
{
  flip32(&(p->img_dim));
  flip32(&(p->img_rows));
  flip32(&(p->img_cols));
  flip32(&(p->no_img_in_file));
  flip32(&(p->pix_bit_allocated));
  flip32(&(p->pix_bit_stored));	
  flip32(&(p->pix_bit_high_pos));
  flip32(&(p->compr_flag));	
  flip32(&(p->x_pix_size));
  flip32(&(p->y_pix_size));
  flip32(&(p->study_slice_thickness));
  flip32(&(p->study_slice_spacing));
  flip32(&(p->subhdr_blk_size));
  flip32(&(p->subhdr_blk_position));
};

#endif /* !__misc_h */
