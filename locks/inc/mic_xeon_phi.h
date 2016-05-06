#ifndef MIC_XEON_PHI_H
#define MIC_XEON_PHI_H

#ifdef __MIC__
#define busy_wait(n, i) _mm_delay_64(n)
#else
#define busy_wait(n, i) for (i = 0; i < n; i++) _mm_pause();
#endif

#endif /* MIC_XEON_PHI_H */
