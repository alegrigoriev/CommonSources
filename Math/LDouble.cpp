#include "LDouble.h"
// true long double arithmetics
#undef NAKED
#define NAKED __declspec(naked)
#pragma warning(disable:4035)
ldouble::ldouble(double a)
{
	_asm {
		mov     eax,dword ptr this
		fld     [a]
		fstp    tbyte ptr [eax]
	}
}

NAKED ldouble __stdcall operator +(const ldouble &a, const ldouble &b)
{
	__asm {
		push	ebx
		push	ecx
		mov	    ecx,8+8[esp]
		mov	    ebx,12+8[esp]
		mov	    eax,4+8[esp]
		fld	    tbyte ptr [ecx]
		pop	    ecx
		fld	    tbyte ptr [ebx]
		fadd
		pop	    ebx
		fstp	tbyte ptr [eax]
		ret	12
	}
}

NAKED ldouble __stdcall operator +(double a, const ldouble &b)
{
	__asm {
		mov	    eax,dword ptr 16[esp]
		fld	    tbyte ptr [eax]
		mov	    eax,dword ptr 4[esp]
		fadd	qword ptr 8[esp]
		fstp	tbyte ptr [eax]
		ret	16
	}
}

NAKED ldouble __stdcall operator +(const ldouble &a, double b)
{
	__asm {
		mov	    eax,dword ptr 8[esp]
		fld	    tbyte ptr [eax]
		mov	    eax,dword ptr 4[esp]
		fadd	qword ptr 12[esp]
		fstp	tbyte ptr [eax]
		ret     16
	}
}

NAKED ldouble __stdcall operator -(const ldouble &a, const ldouble &b)
{
	__asm {
		push	ebx
		push	ecx
		mov	    ecx,16[esp]
		mov	    ebx,20[esp]
		mov	    eax,12[esp]
		fld	    tbyte ptr [ecx]
		pop	    ecx
		fld	    tbyte ptr [ebx]
		fsub
		pop	    ebx
		fstp	tbyte ptr [eax]
		ret	    12
	}
}

NAKED ldouble __stdcall operator -(double a, const ldouble &b)
{
	__asm {
		mov	    eax,dword ptr 16[esp]
		fld	    tbyte ptr [eax]
		mov	    eax,dword ptr 4[esp]
		fsubr	qword ptr 8[esp]
		fstp	tbyte ptr [eax]
		ret     16
	}
}

NAKED ldouble __stdcall operator -(const ldouble &a, double b)
{
	__asm {
		mov	    eax,dword ptr 8[esp]
		fld	    tbyte ptr [eax]
		mov	    eax,dword ptr 4[esp]
		fsub	qword ptr 12[esp]
		fstp	tbyte ptr [eax]
		ret     16
	}
}

NAKED ldouble __stdcall operator *(const ldouble &a, const ldouble &b)
{
	__asm {
		push	ebx
		push	ecx
		mov	    ecx,8+8[esp]
		mov	    ebx,12+8[esp]
		mov	    eax,4+8[esp]
		fld	    tbyte ptr [ecx]
		pop	    ecx
		fld	    tbyte ptr [ebx]
		fmul
		pop	    ebx
		fstp	tbyte ptr [eax]
		ret     12
	}
}

NAKED ldouble __stdcall operator *(double a, const ldouble &b)
{
	__asm {
		mov	    eax,dword ptr 16[esp]
		fld	    tbyte ptr [eax]
		mov	    eax,dword ptr 4[esp]
		fmul	qword ptr 8[esp]
		fstp	tbyte ptr [eax]
		ret     16
	}
}

NAKED ldouble __stdcall operator *(const ldouble &a, double b)
{
	__asm {
		mov	    eax,dword ptr 8[esp]
		fld	    tbyte ptr [eax]
		mov	    eax,dword ptr 4[esp]
		fmul	qword ptr 12[esp]
		fstp	tbyte ptr [eax]
		ret     16
	}
}

NAKED ldouble __stdcall operator /(const ldouble &a, const ldouble &b)
{
	__asm {
		push	ebx
		push	ecx
		mov	    ecx,8+8[esp]
		mov	    ebx,12+8[esp]
		mov	    eax,4+8[esp]
		fld	    tbyte ptr [ecx]
		pop	    ecx
		fld	    tbyte ptr [ebx]
		fdiv
		pop	    ebx
		fstp	tbyte ptr [eax]
		ret     12
	}
}

NAKED ldouble __stdcall operator /(double a, const ldouble &b)
{
	__asm {
		mov	    eax,dword ptr 16[esp]
		fld	    tbyte ptr [eax]
		mov	    eax,dword ptr 4[esp]
		fdivr	qword ptr 8[esp]
		fstp	tbyte ptr [eax]
		ret     16
	}
}

NAKED ldouble __stdcall operator /(const ldouble &a, double b)
{
	__asm {
		mov	    eax,dword ptr 8[esp]
		fld	    tbyte ptr [eax]
		mov	    eax,dword ptr 4[esp]
		fdiv	qword ptr 12[esp]
		fstp	tbyte ptr [eax]
		ret     16
	}
}

NAKED ldouble & ldouble::operator += (const ldouble &a)
{
	__asm {
		mov	    eax,4 [esp]
		fld	    tbyte ptr [ecx]
		fld	    tbyte ptr [eax]
		fadd
		mov	    eax,ecx
		fstp	tbyte ptr [ecx]
		ret     4
	}
}

NAKED ldouble & ldouble::operator += (double a)
{
	__asm {
		fld	    tbyte ptr [ecx]
		fadd	qword ptr 4 [esp]
		mov	    eax,ecx
		fstp	tbyte ptr [ecx]
		ret     8
	}
}

NAKED ldouble & ldouble::operator -= (const ldouble &a)
{
	__asm {
		mov	    eax,4 [esp]
		fld	    tbyte ptr [ecx]
		fld	    tbyte ptr [eax]
		fsub
		mov	    eax,ecx
		fstp	tbyte ptr [ecx]
		ret     4
	}
}

NAKED ldouble & ldouble::operator -= (double a)
{
	__asm {
		fld	    tbyte ptr [ecx]
		fsub	qword ptr 4 [esp]
		mov	    eax,ecx
		fstp	tbyte ptr [ecx]
		ret     8
	}
}

NAKED ldouble & ldouble::operator /= (const ldouble &a)
{
	__asm {
		mov	    eax,4 [esp]
		fld	    tbyte ptr [ecx]
		fld	    tbyte ptr [eax]
		fdiv
		mov	    eax,ecx
		fstp	tbyte ptr [ecx]
		ret     4
	}
}

NAKED ldouble & ldouble::operator /= (double a)
{
	__asm {
		fld	    tbyte ptr [ecx]
		fdiv	qword ptr 4 [esp]
		mov	    eax,ecx
		fstp	tbyte ptr [ecx]
		ret     8
	}
}

NAKED ldouble & ldouble::operator *= (const ldouble &a)
{
	__asm {
		mov	    eax,4 [esp]
		fld	    tbyte ptr [ecx]
		fld	    tbyte ptr [eax]
		fmul
		mov	    eax,ecx
		fstp	tbyte ptr [ecx]
		ret     4
	}
}

NAKED ldouble & ldouble::operator *= (double a)
{
	__asm {
		fld	    tbyte ptr [ecx]
		fmul	qword ptr 4 [esp]
		mov	    eax,ecx
		fstp	tbyte ptr [ecx]
		ret     8
	}
}

NAKED int __stdcall operator==(const ldouble &a, const ldouble &b)
{
	__asm {
		push	ecx
		mov	    eax,8[esp]
		mov	    ecx,12[esp]
		fld	    tbyte ptr [eax]
		fld	    tbyte ptr [ecx]
		fcompp

		fnstsw	ax
		and	    eax,4000h
		pop	    ecx
		shr	    eax,14
		ret	    8
	}
}

NAKED int __stdcall operator==(double a, const ldouble &b)
{
	__asm {
		mov	    eax,12[esp]
		fld	    tbyte ptr [eax]
		fcomp	qword ptr 4[esp]

		fnstsw	ax
		and	    eax,4000h
		shr	    eax,14
		ret     12
	}
}

NAKED int __stdcall operator==(const ldouble &a, double b)
{
	__asm {
		mov	    eax,4[esp]
		fld	    tbyte ptr [eax]
		fcomp	qword ptr 8[esp]

		fnstsw	ax
		and	    eax,4000h
		shr	    eax,14
		ret     12
	}
}

NAKED int __stdcall operator!=(const ldouble &a, const ldouble &b)
{
	__asm {
		push	ecx
		mov	    eax,8[esp]
		mov	    ecx,12[esp]
		fld	    tbyte ptr [eax]
		fld	    tbyte ptr [ecx]
		fcompp

		fnstsw	ax
		and	    eax,4000h
		shr	    eax,14
		pop	    ecx
		xor	    eax,1
		ret     8
	}
}

NAKED int __stdcall operator!=(double a, const ldouble &b)
{
	__asm {
		mov	    eax,12[esp]
		fld	    tbyte ptr [eax]
		fcomp	qword ptr 4[esp]

		fnstsw	ax
		and	    eax,4000h
		shr	    eax,14
		xor	    eax,1
		ret     12
	}
}

NAKED int __stdcall operator!=(const ldouble &a, double b)
{
	__asm {
		mov	    eax,4[esp]
		fld	    tbyte ptr [eax]
		fcomp	qword ptr 8[esp]

		fnstsw	ax
		and	    eax,4000h
		shr	    eax,14
		xor	    eax,1
		ret     12
	}
}

NAKED int __stdcall operator>(const ldouble &a, const ldouble &b)
{
	__asm {
		push	ecx
		mov	    eax,8[esp]
		mov	    ecx,12[esp]
		fld	    tbyte ptr [ecx]
		fld	    tbyte ptr [eax]
		fcompp

		fnstsw	ax
		test	ah,65
		pop	    ecx
		mov	    eax,0
		setz	al
		ret     8
	}
}

NAKED int __stdcall operator>(double a, const ldouble &b)
{
	__asm {
		mov	    eax,12[esp]
		fld	    tbyte ptr [eax]
		fld	    qword ptr 4[esp]
		fcompp

		fnstsw	ax
		test	ah,65
		mov	    eax,0
		setz	al
		ret     12
	}
}

NAKED int __stdcall operator>(const ldouble &a, double b)
{
	__asm {
		mov	    eax,4[esp]
		fld	    tbyte ptr [eax]
		fcomp	qword ptr 8[esp]

		fnstsw	ax
		test	ah,65
		mov	    eax,0
		setz	al
		ret     12
	}
}

NAKED int __stdcall operator<(const ldouble &a, const ldouble &b)
{
	__asm {
		push	ecx
		mov	    eax,12[esp]
		mov	    ecx,8[esp]
		fld	    tbyte ptr [ecx]
		fld	    tbyte ptr [eax]
		fcompp

		fnstsw	ax
		test	ah,65
		pop	    ecx
		mov	    eax,0
		setz	al
		ret     8
	}
}

NAKED int __stdcall operator<(double a, const ldouble &b)
{
	__asm {
		mov	    eax,12[esp]
		fld	    qword ptr 4[esp]
		fld	    tbyte ptr [eax]
		fcompp

		fnstsw	ax
		test	ah,65
		mov	    eax,0
		setz	al
		ret     12
	}
}

NAKED int __stdcall operator<(const ldouble &a, double b)
{
	__asm {
		mov	    eax,4[esp]
		fld	    tbyte ptr [eax]
		fld	    qword ptr 8[esp]
		fcompp

		fnstsw	ax
		test	ah,65
		mov	    eax,0
		setz	al
		ret     12
	}
}

NAKED int __stdcall operator>=(const ldouble &a, const ldouble &b)
{
	__asm {
		push	ecx
		mov	    eax,12[esp]
		mov	    ecx,8[esp]
		fld	    tbyte ptr [ecx]
		fld	    tbyte ptr [eax]
		fcompp

		fnstsw	ax
		test	ah,65
		pop	    ecx
		mov	    eax,0
		setnz	al
		ret     8
	}
}

NAKED int __stdcall operator>=(double a, const ldouble &b)
{
	__asm {
		mov	    eax,12[esp]
		fld	    qword ptr 4[esp]
		fld	    tbyte ptr [eax]
		fcompp

		fnstsw	ax
		test	ah,65
		mov	    eax,0
		setnz	al
		ret     12
	}
}

NAKED int __stdcall operator>=(const ldouble &a, double b)
{
	__asm {
		mov	    eax,4[esp]
		fld	    tbyte ptr [eax]
		fld	    qword ptr 8[esp]
		fcompp

		fnstsw	ax
		test	ah,65
		mov	    eax,0
		setnz	al
		ret     12
	}
}

NAKED int __stdcall operator<=(const ldouble &a, const ldouble &b)
{
	__asm {
		push	ecx
		mov	    eax,8[esp]
		mov	    ecx,12[esp]
		fld	    tbyte ptr [ecx]
		fld	    tbyte ptr [eax]
		fcompp

		fnstsw	ax
		test	ah,65
		pop	    ecx
		mov	    eax,0
		setnz	al
		ret     8
	}
}

NAKED int __stdcall operator<=(double a, const ldouble &b)
{
	__asm {
		mov	    eax,12[esp]
		fld	    tbyte ptr [eax]
		fld	    qword ptr 4[esp]
		fcompp

		fnstsw	ax
		test	ah,65
		mov	    eax,0
		setnz	al
		ret     12
	}
}

NAKED int __stdcall operator<=(const ldouble &a, double b)
{
	__asm {
		mov	    eax,4[esp]
		fld	    tbyte ptr [eax]
		fcomp	qword ptr 8[esp]

		fnstsw	ax
		test	ah,65
		mov	    eax,0
		setnz	al
		ret     12
	}
}

NAKED ldouble::operator double() const
{
	__asm {
		fld	    tbyte ptr [ecx]
		ret     0
	}
}

NAKED ldouble ldouble::operator +() const
{
	__asm {
		push    ebx
		push    edx
		mov     ebx,dword ptr 12[esp]
		mov     eax,[ebx]
		mov     edx,4[ebx]
		mov     [ecx],eax
		mov     4[ecx],edx
		mov     ax,8[ebx]
		pop     edx
		mov     8[ecx],ax
		pop     ebx
		mov     eax,ecx
		ret     4
	}
}

NAKED ldouble ldouble::operator -() const
{
	__asm {
		fld	    tbyte ptr [ecx]
		mov	    eax,DWORD PTR 4[esp]
		fchs
		fstp	tbyte ptr [eax]
		ret     4
	}
}

NAKED ldouble __stdcall sqrt(const ldouble & a)
{
	__asm {
		mov	    eax,DWORD PTR 8[esp]
		fld	    tbyte ptr [eax]
		mov	    eax,DWORD PTR 4[esp]
		fsqrt
		fstp	tbyte ptr [eax]
		ret     8
	}
}
