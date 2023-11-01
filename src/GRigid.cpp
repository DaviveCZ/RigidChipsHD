#include "GVector.hpp"
#include "GRigid.hpp"
#include "GParticle.hpp"
#include <stdio.h>
//���������[�N���o�p
#include <crtdbg.h>  
#ifdef _DEBUG 
#ifdef __MEMLEAKCHECK 
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__) 
#define malloc(p1) _malloc_dbg((p1),_NORMAL_BLOCK,__FILE__,__LINE__) 
#define __MEMLEAKCHECK
#endif
#endif 

GVector VecX=GVector(1,0,0);
GVector VecY=GVector(0,1,0);
GVector VecZ=GVector(0,0,1);

//int GDTSTEP=(int)(10*(0.6f/CHIPSIZE)+0.1);
int GDTSTEP=10;
int GLOOP=10;
int LIMITFPS=30;
extern GFloat GSPEEDLIMIT;
extern GParticle *GroundParticle;
extern GParticle *WaterLineParticle;
extern GParticle *JetParticle;
extern GBullet *Bullet;
//extern bool ObjectBallFlag;
extern bool ShowGhost;
extern DWORD ShowDustFlag;
extern int TickCount;
extern int SystemTickCount;

extern bool EfficientFlag;
extern GFloat TotalPower;

extern double ARM_EFF;
extern double JET_EFF;
extern double WHL_EFF;


void AttackDataDisp(char *s,DWORD dpnid,int attack);
static long myrandval = 1L;
int myRand(){
//	return 500;
	return(((myrandval = myrandval * 214013L + 2531011L) >> 16) & 0x7fffL);
}
void MySrand (unsigned int seed)
{
	myrandval = (long)seed;
}
int myrand();
//***************************************************************
//	���̃N���X���\�b�h
//***************************************************************
//************************************************
//�R���X�g���N�^
//************************************************
GRigid::GRigid(int type,bool fix,GFloat x,GFloat y,GFloat z) {
	Shape.PointN=0;
	Crush=false;
	CheckShape.PointN=0;
	SaveShape.PointN=0;
	Type=type;
	Fixed=fix;
	Ghost=0;
	Model=NULL;
	Param=GVector(x,y,z);
	Density=1000;
	CheckShape=Shape;
	World=NULL;
	ChildCount=0;
	Parent=NULL;
	LinkInfo=NULL;
	Top=this;
	ByeFlag=0;
	CowlTop=this;
	Name[0]='\0';
	Power=0.0f;
	Power2=0.0f;
	PowerSave=0.0f;
	FrameFlag=0;
	Dir=0;
	MeshNo=0;
	FrameFlag=0;
	Option=0;
	Effect=0;
	UserOption=0;
	UserEffect=0;
	DirCode=0;
	Color=0xffffff;
	ChipType=0;
	Energy=0;
	ArmEnergy=250000;
	for(int i=0;i<GCHILDMAX;i++) {
		Child[i].ID=i;
		Child[i].RigidB=NULL;
		Child[i].RigidB2=NULL;
		Child[i].Angle=0;
		Child[i].Axis=GVector(0,1,0);
		Child[i].DamperK=0.5f;
		Child[i].SpringK=1.0f;
		Child[i].FrictionK=0.0f;
	}
	E=1;		//�e�݌W��
	Us=1;		//�Ö��C�W��
	Ud=1;		//�����C�W��
	Ux=0;		//���낪�薀�C�W��
	Cd=1;
	HitN=0;
	HitObj=0;
	HitChip=0;
	HitLand=0;
	HitBullet=0;
	FuelMax=0;
	Fuel=0;
	Reset();

	CalcTotalCenter();
	CreateViewModel();
}
//************************************************
//�f�X�g���N�^
//************************************************
GRigid::~GRigid() {
	//Model�Ƀ����N�����\���f�[�^�͐�ɉ�����邱��
}

//************************************************
// ���́F�����̃��Z�b�g
//************************************************
void GRigid::Reset()
{
	Shape.PointN=0;
	//	CheckShape.PointN=0;
	X.clear();
	V.clear();
	W.clear();
	P.clear();
	L.clear();
	P2.clear();
	L2.clear();
	R=GMatrix();
	preX.clear();
	preV.clear();
	Crush=false;
	TM=R.translate(X);
	SetTensor();
	Energy=0;
	Fuel=FuelMax;
	Tolerant=10000;
	preF=0;
	Rt=R.transpose();
	Ext=GVector(0,0,0);
	//�p�����ς��΃e���\�����ς��
   	I_ = Rt*Ib_*R;		//OK
	I = I_.inverse();		//�����e���\���̋t��
	TotalFuelMax=0;
	TotalFuel=0;
	TotalFuel2=0;
}
//************************************************
// R���Z�b�g����
//************************************************
void GRigid::RSet()
{
	HitN=0;
	TM=R.translate(X);
	if(MeshNo==23) return;
	Rt=R.transpose();
	Q=R.quat();
	preV=GVector(0,0,0);
	//�p�����ς��΃e���\�����ς��
   	I_ = Rt*Ib_*R;		//OK
	I = I_.inverse();		//�����e���\���̋t��
	
}
void GRigid::CalcTotalFuel()
{
	if(Parent==NULL) {
		Top->TotalFuelMax=0;
		Top->TotalFuel=0;
		Top->TotalFuel2=0;
	}
	Top->TotalFuelMax+=FuelMax;
	Top->TotalFuel+=Fuel;
	if(ChipType==GT_CHIPH) Top->TotalFuel2+=Fuel;
	for(int i=0;i<ChildCount;i++) {
		if(Child[i].RigidB!=NULL) {
			Child[i].RigidB->CalcTotalFuel();
		}
	}
}
double GRigid::CheckFuel(double f)
{
	if(Top->TotalFuel>=fabs(f)) return f;
	if(f>=0) return Top->TotalFuel;
	return -Top->TotalFuel;
}
void GRigid::UseFuel(double f)
{
	double rate=1.0f;
	if(Top->TotalFuel2>0) {
		rate=fabs(f)/Top->TotalFuel2;
		if(ChipType==GT_CHIPH) {
			if(rate>1) Fuel=0;
			Fuel-=(Fuel*rate);
			if(Fuel<=0) Fuel=0;
		}
		for(int i=0;i<ChildCount;i++) {
			if(Child[i].RigidB!=NULL) {
				Child[i].RigidB->UseFuel(f);
			}
		}
	}
	else if(Top->TotalFuel>0) {
		rate=fabs(f)/Top->TotalFuel;
		if(ChipType!=GT_CHIPH) {
			if(rate>1) Fuel=0;
			Fuel-=(Fuel*rate);if(Fuel<=0) Fuel=0;
		}
		for(int i=0;i<ChildCount;i++) {
			if(Child[i].RigidB!=NULL) {
				Child[i].RigidB->UseFuel(f);
			}
		}
	}
}
void GRigid::CalcTotalCenter(void)
{
	GVector x;
	GVector xg;
	int c=1,cc=0;
	int cg=1;
	x=X;
	xg=X*M;
	Top->TotalMass=M;
	for(int i=0;i<ChildCount;i++) {
		if(Child[i].RigidB!=NULL ) {
			x+=Child[i].RigidB->CalcTotalPos(&c,&cc);
			if(Child[i].RigidB->ChipType!=GT_COWL) {
				xg+=Child[i].RigidB->CalcTotalXG(&cg);
				Child[i].RigidB->CalcTotalMass();
			}	
		}
	}
	TotalCenter=x/(GFloat)c;
	TotalCount=c;
	TotalCowlCount=cc;
	GFloat m=Top->TotalMass;
	if(m==0) m=1.0f;
	TotalCenterOfGravity=xg/m;
	TotalRadius=CalcTotalRadius(0.0f);
}
GVector GRigid::CalcTotalPos(int *c,int *cc)
{
	GVector x;
	x=X;
	for(int i=0;i<ChildCount;i++) {
		if(Child[i].RigidB!=NULL) {
			x+=Child[i].RigidB->CalcTotalPos(c,cc);
		}
	}
	if(MeshNo==23) (*cc)++;
	(*c)++;
	return x;
}
void GRigid::CalcTotalMass()
{
	Top->TotalMass+=M;
	for(int i=0;i<ChildCount;i++) {
		if(Child[i].RigidB!=NULL) {
			Child[i].RigidB->CalcTotalMass();
		}
	}
}
GVector GRigid::CalcTotalXG(int *c)
{
	GVector xg;
	xg=X*M;
	for(int i=0;i<ChildCount;i++) {
		if(Child[i].RigidB!=NULL) {
			xg+=Child[i].RigidB->CalcTotalXG(c);
		}
	}
	(*c)++;
	return xg;
}

GFloat GRigid::CalcTotalRadius(GFloat r)
{
	GFloat a=(X-Top->X).abs()+Radius;
	if(a>r) r=a;
	for(int i=0;i<ChildCount;i++) {
		if(Child[i].RigidB!=NULL) {
			r=Child[i].RigidB->CalcTotalRadius(r);
		}
	}
	return r;
}
int GRigid::CountTotal()
{
	int c=1;
	int i;	
	for(i=0;i<ChildCount;i++) {
		if(Child[i].RigidB) {
			c+=Child[i].RigidB->CountTotal();
		}
	}
	return c;
}
//************************************************
// �e�q�֌W�ƂƂ��Ɉړ�����
//************************************************
void GRigid::RotateWithChildAbs(GMatrix33 m,GVector x)
{
	int i;
	GMatrix33 rm=R.transpose()*m;
	R=R*rm;
	X=x+(X-x)*rm;
	RSet();
	
	for(i=0;i<ChildCount;i++) 
		if(Child[i].RigidB) Child[i].RigidB->RotateWithChild(rm,x);
}
void GRigid::RotateWithChild(GMatrix33 m,GVector x)
{
	int i;
	R=R*m;
	X=x+(X-x)*m;
	RSet();
	
	for(i=0;i<ChildCount;i++) 
		if(Child[i].RigidB) Child[i].RigidB->RotateWithChild(m,x);
}
void GRigid::RotateWithChild(GMatrix33 m,GVector x,GRigid* noUse)
{
	if(this==noUse) return;
	int i;
	R=R*m;
	X=X*(GMatrix().translate(-x)*m*GMatrix().translate(x));
	
	for(i=0;i<ChildCount;i++)
		if(Child[i].RigidB) Child[i].RigidB->RotateWithChild(m,x,noUse);
		RSet();
}
void GRigid::RotateWithParentAll2(GMatrix33 m,GVector x)
{
	//��Ԃ̐e��������
	GRigid *r=this;
	while(r->Parent) r=r->Parent;
	r->RotateWithChild(m,x,this);
}
void GRigid::ResetXfWithChild()
{
	preX=X;
	for(int i=0;i<ChildCount;i++)
		if(Child[i].RigidB) Child[i].RigidB->ResetXfWithChild();
}
void GRigid::EnervateWithChild()
{
	P.clear();
	L.clear();
	V.clear();
	W.clear();
	for(int i=0;i<ChildCount;i++)
		if(Child[i].RigidB) Child[i].RigidB->EnervateWithChild();
}
void GRigid::TranslateWithChild(GVector &v)
{
	TM=R.translate(X+=v);
	for(int i=0;i<ChildCount;i++)
		if(Child[i].RigidB && Child[i].RigidB->ChipType!=GT_COWL) Child[i].RigidB->TranslateWithChild(v);
}
void GRigid::TranslateWithChild(GVector v,GRigid* noUse)
{
	if(this==noUse) return;
	int i;
	X+=v;
	TM=R.translate(X);
	
	for(i=0;i<ChildCount;i++) {
		if(Child[i].RigidB && Child[i].RigidB->ChipType!=GT_COWL) Child[i].RigidB->TranslateWithChild(v,noUse);
	}
}
void GRigid::TranslateWithParentAll(GVector v)
{
	//��Ԃ̐e��������
	GRigid *r=this;
	while(r->Parent) r=r->Parent;
	r->TranslateWithChild(v);	
}

void GRigid::TranslateWithParent(GVector v)
{
	Parent->X+=v;
	TranslateWithChild(v);
	Parent->TM=Parent->R.translate(Parent->X);
	
}
void GRigid::TranslateWithParentAll2(GVector v)
{
	//��Ԃ̐e��������
	GRigid *r=this;
	while(r->Parent) r=r->Parent;
	r->TranslateWithChild(v,this);	
}
//************************************************
// ���́F�����e���\���̐���
//************************************************
void GRigid::SetTensor(){
	int i;
	GFloat x,y,z;
	GFloat x2,y2,z2;
	GFloat w;
	x=Param.x;y=Param.y;z=Param.z;
	x2=x/(GFloat)2.0;y2=y/(GFloat)2.0;z2=z/(GFloat)2.0;
	Volume=0.0f;
	//��{�`��̊����e���\���ꗗ
	Ib=GMatrix33();
	switch(Type) {
	case GTYPE_FACE://X:x, Z:z �̕���
		M=Density*x*y*z;
		Volume=M/Density;
		Ib.elem[0][0]=M*(z*z+y*y)/(GFloat)12.0; 
		Ib.elem[1][1]=M*(z*z+x*x)/(GFloat)12.0; 
		Ib.elem[2][2]=M*(x*x+y*y)/(GFloat)12.0; 
		//�`��̍쐬
		Shape.PointN=9;
		Shape.Point[0]=GVector(x2,0,0);
		Shape.Point[1]=GVector(0,0,-z2);
		Shape.Point[2]=GVector(-x2,0,0);
		Shape.Point[3]=GVector(0,0,z2);
		Shape.Point[4]=GVector(x2,0,z2);
		Shape.Point[5]=GVector(x2,0,-z2);
		Shape.Point[6]=GVector(-x2,0,-z2);
		Shape.Point[7]=GVector(-x2,0,z2);
		Shape.Point[8]=GVector(0,0,0);
		w=Param.Max()/2.0f;
		Radius=(GFloat)sqrt(w*w*3);
		break;
	case GTYPE_DISK://���ax����y�̉~��
		M=(GFloat)(Density*M_PI*x2*x2*y);
		Volume=M/Density;
		Ib.elem[0][0]=M*(x2*x2/4+y*y/12); 
		Ib.elem[1][1]=M*x2*x2/2,0;
		Ib.elem[2][2]=M*(x2*x2/4+y*y/12); 
		//�`��̍쐬
#define N 12
		Shape.PointN=N+1;
		Radius=(Param.x>Param.z?Param.x:Param.z)/(GFloat)2.0;
		for(i=0;i<N;i++) {
			Shape.Point[i]=GVector((GFloat)(cos(M_PI*2*i/N))*Radius,(GFloat)0,(GFloat)(sin(M_PI*2*i/N)*Radius));
		}
		Shape.Point[N]=GVector(0,0,0);
		break;
	case GTYPE_BOX://X:x, Y:y, Z:z �̒�����
		M=Density*x*y*z;
		Volume=M/Density;
		Ib.elem[0][0]=M*(y*y+z*z)/(GFloat)12.0; 
		Ib.elem[1][1]=M*(z*z+x*x)/(GFloat)12.0; 
		Ib.elem[2][2]=M*(x*x+y*y)/(GFloat)12.0; 
		//�`��̍쐬
		Shape.PointN=9;
		Shape.Point[0]=GVector(x2,y2,z2);
		Shape.Point[1]=GVector(x2,y2,-z2);
		Shape.Point[2]=GVector(-x2,y2,-z2);
		Shape.Point[3]=GVector(-x2,y2,z2);
		Shape.Point[4]=GVector(x2,-y2,z2);
		Shape.Point[5]=GVector(x2,-y2,-z2);
		Shape.Point[6]=GVector(-x2,-y2,-z2);
		Shape.Point[7]=GVector(-x2,-y2,z2);
		Shape.Point[8]=GVector(0,0,0);
		w=Param.Max()/(GFloat)2.0;
		Radius=(GFloat)sqrt(w*w*3);
		break;
	case GTYPE_DUMMY:			
	case GTYPE_BALL://���ax,y,z�̑ȉ~��
		M=(GFloat)(Density*4.0f/3.0f*M_PI*x2*y2*z2);
		Volume=M/Density;
		Ib.elem[0][0]=(GFloat)(M*2.0/5.0*y2*z2);
		Ib.elem[1][1]=(GFloat)(M*2.0/5.0*z2*x2); 
		Ib.elem[2][2]=(GFloat)(M*2.0/5.0*x2*y2);
		//�`��̍쐬
		Shape.PointN=N+1;
		for(i=0;i<N;i++) {
			Shape.Point[i]=GVector((GFloat)(x2*cos(M_PI*2*i/N)),(GFloat)0,(GFloat)(z2*sin(M_PI*2*i/N)));
		}
		Shape.Point[N]=GVector(0,0,0);
		Radius=Param.Max()/(GFloat)2.0;
		break;
		
	case GTYPE_CAN://���ax����y�̉~��
		M=(GFloat)(Density*M_PI*x2*x2*y);
		Volume=M/Density;
		Ib.elem[0][0]=(GFloat)(M*(x2*x2/4+y*y/12)); 
		Ib.elem[1][1]=(GFloat)(M*x2*x2/2,0);
		Ib.elem[2][2]=(GFloat)(M*(x2*x2/4+y*y/12)); 
		Radius=(GFloat)sqrt(x2*x2+y2*y2);
		break;
		
	case GTYPE_CONE://���ar����y�̉~��
		M=(GFloat)(Density*M_PI*x2*x2*y/3.0f);
		Volume=M/Density;
		//�d�S�F����3/4�̈ʒu 
		Ib.elem[0][0]=(GFloat)(M*3/20.0*(x2*x2+y*y/4));
		Ib.elem[1][1]=(GFloat)(M*3/10.0*x2*x2);
		Ib.elem[2][2]=(GFloat)(M*3/20.0*(x2*x2+y*y/4));
		Radius=(GFloat)sqrt(x2*x2+y2*y2);
		break;
	}
	M_=(GFloat)1.0/M;
	Ib_=Ib.inverse();
}


//************************************************
// ���́F����(�^����)��������
//************************************************
void GRigid::ApplyImpulse(GVector &momentum,GVector &pos)
{
	
	P += momentum;
	GVector l = (pos-X).cross(momentum);
	L+=l;
	P2+=momentum;
	L2+=l;
}
void GRigid::ApplyImpulse2(GVector &momentum,GVector &pos)
{
	
	P += momentum;
	GVector l = pos.cross(momentum);
	L+=l;
	P2+=momentum;
	L2+=l;
}


//************************************************
// ���́F����(�^����)��������
//************************************************
void GRigid::ApplyAngularImpulse(GVector momentum)
{
	L += momentum;
	L2+=momentum;
}
//************************************************
// ���́F�͂�������
//************************************************
void GRigid::ApplyForce(GVector force,GVector pos)
{
	GVector f=force*World->Dt;
	P += f;
	GVector l = (pos-X).cross(f);
	L+=l;
	P2+=f;
	L2+=l;
	
}
//************************************************
// ���́F�g���N��������
//************************************************
void GRigid::ApplyTorque(GVector torque)
{
	GVector l= torque*World->Dt;
	L+=l;
	L2+=l;
}
//************************************************
// ���́F���͂�������(���[�J��)
//************************************************
void GRigid::ApplyLocalImpulse(GVector momentum,GVector pos)
{
	GVector f=momentum*R;
	P += f;
	GVector l= (pos*R).cross(f);
	L+=l;
	P2+=f;
	L2+=l;
}
//************************************************
// ���́F�p�^���ʂ�������(���[�J��)
//************************************************
void GRigid::ApplyLocalAngularImpulse(GVector momentum)
{
	GVector l = momentum*R;
	L+=l;
	L2+=l;
}
//************************************************
// ���́F�͂�������(���[�J��)
//************************************************
void GRigid::ApplyLocalForce(GVector force,GVector pos)
{
	GVector p=force*R*World->Dt;
	P += p;
	GVector l = (pos*R).cross(p);
	L+=l;
	P2+=p;
	L2+=l;
}
//************************************************
// ���́F�g���N��������(���[�J��)
//************************************************
void GRigid::ApplyLocalTorque(GVector torque)
{
	GVector l= torque*R*World->Dt;
	L+=l;
	L2+=l;
}
//************************************************
// ���́F�����蔻��
//************************************************
void GRigid::CollisionCheck(GRigid *rigid)
{
	int i,j;
	if(rigid->Ghost==1 || Ghost==1) {
		if(rigid->Type==-1) {
			for(i=0;i<ChildCount;i++) {
				if(Child[i].RigidB) Child[i].RigidB->CollisionCheck(rigid);
			}
		}
		return;
	}
	GVector r,v;
	GFloat d;
	GFloat ud,us;
	bool hitFlag=false;
	GVector normal;
	GFloat gr=World->G.abs()*World->Dt/2.0f;
	if(rigid->Type==-1) { //�n�ʃf�[�^
				World->Land->List0up(X,Radius+gr+V.abs()*World->Dt); //World->Land->Check2�Ɠ��� +gr�͂����炭�K���萔 0���ƌ덷�Ŋђʂ���Ƃ͎v��
		switch (Type) {
			case GTYPE_FACE:
				if(MeshNo==23) {
				}
				else {
					GVector v2=V*World->Dt;
					GFloat v2_abs=v2.abs();
					if(v2_abs*20 < CHIPSIZE/1.0f) v2=v2*20; //��אH���y��
					else if(v2_abs*4 < CHIPSIZE/1.0f) v2=v2/v2_abs*CHIPSIZE/1.0f;
					else v2=v2*4;
					for(j=0;j<CheckShape.PointN;j++) {
						GVector p=(CheckShape.Point[j])*TM;
						GVector v3=v2+(p-X)/2;
						d=World->Land->Check(p,v3,v2_abs,normal,ud,us);
						if(d<0 && (X-p).dot(normal) > -0.02f) { //-0.02: 91.15deg
							Hit[HitN].CheckShapeNo=j;
							Hit[HitN].Pos=p;
							Hit[HitN].Normal=normal;
							Hit[HitN].Distance=d;
							Hit[HitN].Target=rigid;
							Hit[HitN].Ud=ud;
							Hit[HitN].Us=us;
							Hit[HitN].Ux=0.0f;
							HitN++;
							hitFlag=true;
						}
					}
				}
				break;
			case GTYPE_BOX:
				for(j=0;j<CheckShape.PointN;j++) {
					GVector p=(CheckShape.Point[j])*TM;
					if((d=World->Land->Check(p,(p-X).normalize2(),1.0f,normal,ud,us))<0) {
						Hit[HitN].CheckShapeNo=j;
						Hit[HitN].Pos=p;
						Hit[HitN].Normal=normal;
						Hit[HitN].Distance=d;
						Hit[HitN].Target=rigid;
						Hit[HitN].Ud=ud;
						Hit[HitN].Us=us;
						Hit[HitN].Ux=0.0f;
						HitN++;
						hitFlag=true;
					}
				}
				break;
			case GTYPE_BALL:
				RSet();
				r=Param/2;
				//World->Land->Check2(X,Radius+gr);
				for(int k=0;k<World->Land->List0Count;k++) {
					normal=World->Land->Face[World->Land->List0[k]].Normal;
					GVector  p;
					d=World->Land->Face[World->Land->List0[k]].BallCheck(X,Radius,p);
					//				if(v.y>0) v=-v;
					if(d<=0) {
//						if(d>0) continue;
						Hit[HitN].CheckShapeNo=-1;
						Hit[HitN].Pos=p;
						Hit[HitN].Normal=(X-p).normalize2();
						Hit[HitN].Distance=d;
						Hit[HitN].Target=rigid;
						Hit[HitN].Ud=World->Land->Face[World->Land->List0[k]].Ud;
						Hit[HitN].Us=World->Land->Face[World->Land->List0[k]].Us;
						Hit[HitN].Ux=World->Land->Face[World->Land->List0[k]].Ux;
						HitN++;
						hitFlag=true;
					}
				}
				break;
			case GTYPE_DISK:
				{
				RSet();
				//World->Land->Check2(X,Radius+gr);
				GVector n=GVector(R.elem[1][0],R.elem[1][1],R.elem[1][2]);
					GVector v2=V*World->Dt;
					GFloat v2_abs=v2.abs();
					if(v2_abs*10 < CHIPSIZE/1.0f) v2=v2*10; //��אH���y��
					else if(v2_abs*4 < CHIPSIZE/1.0f) v2=v2/v2_abs*CHIPSIZE/1.0f;
					else v2=v2*4;
				for(int k=0;k<World->Land->List0Count;k++) {
					
					GVector n2=World->Land->Face[World->Land->List0[k]].Normal;
					GVector nr=n2.cross(n).cross(n).normalize2();
					if(nr.dot(n2)>0) nr=-nr;
					GVector p=X+nr*Radius; //�ڐG�p��ʖ@���޸�قŌ��ߑł�����ƕӂƐڐG����ꍇ����肱�ڂ�
//					GVector v2=V*World->Dt*10+W.cross(p-X)*World->Dt*10;
					GVector v3=v2+(p-X)/2;
					if((d=World->Land->Face[World->Land->List0[k]].Check3(p,v3,0,0,normal))<0) {
						Hit[HitN].CheckShapeNo=-1;
						Hit[HitN].Pos=p;
						Hit[HitN].Normal=normal;
						Hit[HitN].Distance=d;
						Hit[HitN].Target=rigid;
						Hit[HitN].Ud=World->Land->Face[World->Land->List0[k]].Ud;
						Hit[HitN].Us=World->Land->Face[World->Land->List0[k]].Us;
						Hit[HitN].Ux=World->Land->Face[World->Land->List0[k]].Ux;
						HitN++;
						hitFlag=true;
					}
				}
				for(j=0;j<CheckShape.PointN;j++) {
					GVector p=(CheckShape.Point[j])*TM;
//					GVector v2=V*World->Dt*10+W.cross(p-X)*World->Dt*10;
					GVector v3=v2+(p-X)/2;
					d=World->Land->Check(p,v3,v2_abs,normal,ud,us);
					if(d<0 && (X-p).dot(normal) > -0.02f) {
						Hit[HitN].CheckShapeNo=j;
						Hit[HitN].Pos=p;
						Hit[HitN].Normal=normal;
						Hit[HitN].Distance=d;
						Hit[HitN].Target=rigid;
						Hit[HitN].Ud=ud;
						Hit[HitN].Us=us;
						Hit[HitN].Ux=0.0f;
						HitN++;
						hitFlag=true;
					}
				}

				break;
				}
				
			case GTYPE_CAN:
				RSet();
				r=Param/2;r.y=0;
				GVector a=GVector(0,Param.y/2,0)*Rb*R;
				GVector p=X+a;
				if((d=World->Land->Check(p,(p-X).normalize2(),1.0f,normal,ud,us))<=0) {
					Hit[HitN].CheckShapeNo=-1;
					Hit[HitN].Pos=p;
					Hit[HitN].Normal=normal;
					Hit[HitN].Distance=d;
					Hit[HitN].Target=rigid;
					Hit[HitN].Ud=ud;
					Hit[HitN].Us=us;
					Hit[HitN].Ux=0.0f;
					HitN++;
					hitFlag=true;
				}
				p=X-a;
				if((d=World->Land->Check(p,V.normalize2(),1.0f,normal,ud,us))<=0) {
					Hit[HitN].CheckShapeNo=-1;
					Hit[HitN].Pos=p;
					Hit[HitN].Normal=normal;
					Hit[HitN].Distance=d;
					Hit[HitN].Target=rigid;
					Hit[HitN].Ud=ud;
					Hit[HitN].Us=us;
					Hit[HitN].Ux=0.0f;
					HitN++;
					hitFlag=true;
				}
				d=World->Land->Check((X+a),V.normalize2(),1.0f,normal,ud,us);
				v=(-normal*Rt*Rb.transpose()*r).normalize()*r*Rb*R;
				//				if(v.y>0) v=-v;
				if(d<=v.abs()+gr) {
					Hit[HitN].CheckShapeNo=-1;
					Hit[HitN].Pos=X+a+v;
					Hit[HitN].Normal=normal;
					Hit[HitN].Distance=d-v.abs();
					Hit[HitN].Target=rigid;
					Hit[HitN].Ud=ud;
					Hit[HitN].Us=us;
					Hit[HitN].Ux=0.0f;
					HitN++;
					hitFlag=true;
				}
				d=World->Land->Check((X-a),V.normalize2(),1.0f,normal,ud,us);
				v=(-normal*Rt*Rb.transpose()*r).normalize()*r*Rb*R;
				if(v.y>0) v=-v;
				if(d<=-v.y+gr) {
					Hit[HitN].CheckShapeNo=-1;
					Hit[HitN].Pos=X-a+v;
					Hit[HitN].Normal=normal;
					Hit[HitN].Distance=d+v.y;
					Hit[HitN].Target=rigid;
					Hit[HitN].Ud=ud;
					Hit[HitN].Us=us;
					Hit[HitN].Ux=0.0f;
					HitN++;
					hitFlag=true;
				}
				break;
		}
		if(hitFlag) {
			Top->TotalHitCount++;
			HitLand++;
		}
		for(i=0;i<ChildCount;i++) {
			if(Child[i].RigidB) Child[i].RigidB->CollisionCheck(rigid);
		}
	}
	else { //������ްقƂ̔���p?
		switch (Type) {
			case GTYPE_FACE:
				if(MeshNo==23) {
/*					for(j=0;j<CheckShape.PointN;j++) {
						GVector p=(CheckShape.Point[j])*TM;
						GVector v=p-rigid->X;
						d=v.abs()-rigid->Radius;
						if(d<=0) {
							CowlTop->Hit[CowlTop->HitN].CheckShapeNo=j;
							CowlTop->Hit[CowlTop->HitN].Pos=p;
							CowlTop->Hit[CowlTop->HitN].Normal=normal;
							CowlTop->Hit[CowlTop->HitN].Distance=d;
							CowlTop->Hit[CowlTop->HitN].Target=rigid;
							CowlTop->HitN++;
							hitFlag=true;
							rigid->Hit[rigid->HitN].CheckShapeNo=-1;
							rigid->Hit[rigid->HitN].Pos=p;
							rigid->Hit[rigid->HitN].Normal=-v.normalize2();
							rigid->Hit[rigid->HitN].Distance=d;
							rigid->Hit[rigid->HitN].Target=CowlTop;
							rigid->HitN++;
							rigid->HitChip++;
						}
					}
*/				}
				else {
					for(j=0;j<CheckShape.PointN;j++) {
						GVector p=(CheckShape.Point[j])*TM;
						GVector v=p-rigid->X;
						d=v.abs()-rigid->Radius;
						if(d<=0) {
							Hit[HitN].CheckShapeNo=j;
							Hit[HitN].Pos=p;
							Hit[HitN].Normal=v.normalize2();
							Hit[HitN].Distance=d;
							Hit[HitN].Target=rigid;
							HitN++;
							hitFlag=true;
							rigid->Hit[rigid->HitN].CheckShapeNo=-1;
							rigid->Hit[rigid->HitN].Pos=p;
							rigid->Hit[rigid->HitN].Normal=-v.normalize2();
							rigid->Hit[rigid->HitN].Distance=d;
							rigid->Hit[rigid->HitN].Target=this;
							rigid->HitN++;
							rigid->HitChip++;
						}
					}
				}
				break;
			case GTYPE_DISK:
				for(j=0;j<CheckShape.PointN;j++) {
					GVector p=(CheckShape.Point[j])*TM;
					GVector v=p-rigid->X;
					d=v.abs()-rigid->Radius;
					if(d<=0) {
//						if(d>0) d=0;
						Hit[HitN].CheckShapeNo=j;
						Hit[HitN].Pos=p;
						Hit[HitN].Normal=v.normalize2();
						Hit[HitN].Distance=d;
						Hit[HitN].Target=rigid;
						rigid->Hit[rigid->HitN].CheckShapeNo=-1;
						rigid->Hit[rigid->HitN].Pos=p;
						rigid->Hit[rigid->HitN].Normal=-Hit[HitN].Normal;
						rigid->Hit[rigid->HitN].Distance=d;
						rigid->Hit[rigid->HitN].Target=this;
						HitN++;
						rigid->HitN++;
						rigid->HitChip++;
						hitFlag=true;
					}
				}
				break;
			case GTYPE_BALL:
				if(rigid->Type==GTYPE_BALL) {
					GVector v=(X-rigid->X);
					GFloat vl=v.abs();
					d=-(Radius+rigid->Radius-vl)/2;
					if(d<0) {
						Hit[HitN].CheckShapeNo=-1;
						Hit[HitN].Pos=v.normalize2()*Radius+X;
						Hit[HitN].Normal=v.normalize2();
						Hit[HitN].Distance=d;
						Hit[HitN].Target=rigid;
						rigid->Hit[rigid->HitN].CheckShapeNo=-1;
						rigid->Hit[rigid->HitN].Pos=Hit[HitN].Pos;
						rigid->Hit[rigid->HitN].Normal=-Hit[HitN].Normal;
						rigid->Hit[rigid->HitN].Distance=d;
						rigid->Hit[rigid->HitN].Target=this;
						HitN++;
						rigid->HitN++;
						hitFlag=true;
					}
				}
				break;
		}
		if(hitFlag) {
			if(Top) Top->TotalHitCount++;
			HitObj++;
			if(rigid->Top) rigid->Top->TotalHitCount++;
			rigid->HitObj++;
		}
	}
}
//************************************************
// ���́F�ړ�����
//************************************************

void GRigid::Impulse() {
	if(ChipType==GT_COWL) return;
	GVector n,nmin;
	GVector na;
	GVector nb;
	GFloat dmin=0.0;
	GFloat Dt=World->Dt;
	//�����Œ�Ȃ瓮���Ȃ�
	if(Fixed==true) {
		//�V�����ʒu�E�p���p�s��̍쐬
		TM=R.translate(X);
		return;
	}
	//�V�������i�^���ʂƉ�]�^���ʂ����߂�
	//�͂�����i�^���ʂ����߂�
	//�d��
	//				ApplyForce(World->G*M,X);
	//��C��R
	//				P -= P*0.0001*M*Dt;		// ���ʂɊ֌W���Ȃ��悤��m�Ŋ���
	//				L -= L*0.0001*M*Dt;		// ��]�ɂ��������C��R�F�蔲���A���x�ɂ��������C��R�̃X�J���[�{
	
	//�g���N�����]�^���ʂ����߂�
	
	//�Փ˂�����Ό��͂����߂�
	if(HitN>0) {
		GVector hitPos;
		GVector rmax;
		GFloat d=0;
		int hn=HitN;
		//if(HitN>32) hn=32;
		for(int i=0;i<hn;i++) {
			d+=Hit[i].Distance;
			if(dmin>Hit[i].Distance) {
				dmin=Hit[i].Distance;
				nmin=Hit[i].Normal;
			}
		}
		if(d!=0) {
			GVector fvTotal;
			GVector hitPosTotal;
			fvTotal.clear();
			hitPosTotal.clear();
			int hn2=0;
			for(int i=0;i<hn;i++) {
				if(Hit[i].Distance>=0) continue;
				n=Hit[i].Normal;
				hitPos=Hit[i].Pos;
				//�Փ˓_�̏d�S����̈ʒu
				GVector ra= hitPos-X;
				GVector rb;
				//�Փ˓_�ł̑��x
				GVector Va=V+(W.cross(ra));
				GVector Vrel;//���Α��x
				if(Hit[i].Target->Fixed==false) {
					rb = hitPos-Hit[i].Target->X;
					GVector Vb=Hit[i].Target->V+(Hit[i].Target->W.cross(rb));
					Vrel= Va-Vb;//���Α��x
				}
				else {
					Vrel= Va;//���Α��x
				}
				GVector VrelU;//���Α��x
				VrelU=Vrel*(-VecY*R);
				
				//���Α��x���Փ˕����Ɍ����Ă�Ȃ�Փˏ���
				GFloat nv=n.dot(Vrel);
				if(nv>=0) continue;
				GFloat nv2=n.dot(VrelU.normalize2());
				
				GFloat dk=fabs(Hit[i].Distance/d);
				rmax=(Hit[i].Pos-X).Cut2(n);

				//���͂�����i�^���ʂƉ�]�^���ʂ�������
				//���˕Ԃ�W��
				GFloat e=E * Hit[i].Target->E;  //���̒e�˂�����W��
				//���͂̌v�Z
				GFloat j;
				if(Hit[i].Target->Fixed) //�Œ蕨�Ƃ̏Փ�
					j=1.0f/(M_+n.dot((ra.cross(n)*I_).cross(ra)));//OK
				else {//�������Ƃ̏Փ�
					//�Փ˓_�̏d�S����̈ʒu
					j=1.0f/(M_+Hit[i].Target->M_+n.dot((ra.cross(n)*I_).cross(ra))+n.dot((rb.cross(n)*Hit[i].Target->I_).cross(rb)));
				}
				GFloat d2=(1.0f+fabs(Hit[i].Distance));
				GFloat j1=nv*j*dk*dk*d2;
				GVector J=-(1.0f+e)*j1*n;
				//���͂ɂ�鑬�x,�p���x�̕ω�
				if(nv<=0) {
					Hit[i].J=J;
					ApplyImpulse(J,hitPos);
					if((GFloat)fabs(j1)>Top->MaxImpulse)
						Top->MaxImpulse=(GFloat)fabs(j1);
				}
				else {j1=-j1;nv=-nv;}
				//���C�ɂ���R��
				GFloat uk=1.2f;//���񂿂��W��
				GFloat us;
				GFloat ud;
				GFloat ux;
				if(Hit[i].Target->Type==-1) {
					us=Us*Hit[i].Us;
					ud=Ud*Hit[i].Ud*uk;
					ux=Hit[i].Ux;
				}
				else {
					us=Us*Hit[i].Target->Us;
					ud=Ud*Hit[i].Target->Ud*uk;
					ux=Hit[i].Target->Ux;
				}
				//���낪�薀�C
				if((ChipType==GT_WHEEL || ChipType==GT_RLW)&&ux>0) {
					ApplyTorque(-L/Dt*ux); //�����]��R�����  �]�����R����Ȃ����
				}
				//-----------
				GFloat ud2=0.0f;
				
				GVector n2=Vrel.Cut2(n).normalize2();
				GVector n2U=VrelU.Cut2(n).normalize2();
				GFloat j2=n2.dot(Vrel)*j;
				GFloat j2U=n2U.dot(VrelU)*j;
				GVector L2;
				if(HitN>=1) L2=L*n;
				if(nv<0) {
					j2=-j2;
					j1=-j1;
				}
				GFloat dd=1.2f;

				if(Type==GTYPE_DISK) {
					if( (fabs(j2U)+L2.abs())<fabs(j1)*us) {
						GVector fv=j2U*n2U;
						//ApplyImpulse(fv,hitPos);
						fvTotal+=fv;
						//hitPosTotal+=hitPos;
						//hn2++;
						//L=L-L2;
						
					}
					else {
						GVector fv=-j1*(n2U*nv2*ud)*0.5;
						//ApplyImpulse(fv,hitPos);
						fvTotal+=fv;
					}
				}

				if((fabs(j2)+L2.abs())<fabs(j1)*us) {
					GVector fv=j2*n2;
					if(i==0) Hit[i].FricV+=fv*ud;
					else Hit[i].FricV=fv*ud;
					//ApplyImpulse(fv,hitPos);
					fvTotal+=fv;
					hitPosTotal+=hitPos;
					hn2++;
					//L=L-L2;
					
				}
				else {
					if(ShowDustFlag) {
						//
						GFloat a=(j2*n2-j1*(n2*ud+na*ud2)*dd/2.0f).abs();
						if((myrand()%100)<(int)(V.abs()*(1+ux*50)) && a>30.0) {
							a=(a-30.0f)/100.0f+(myrand()%100)/5000.0f-0.01f;
							if(a>1.0f) a=1.0f;else if(a<=0.0f) a=0.0f;
							GVector v=j1*n2/5*Dt/30.0f+GVector(0,0.03f,0);
							if(v.abs()>0.1f) v=v.normalize()/10.0f;
							GroundParticle->Add(hitPos-V*Dt*(float)(rand()%8),v,GVector(0,0,0),v.abs()*(1+ux*50),a,0.02f+(rand()%10)/100.0f,GVector(1,1,1));
						}
					}
					GVector fv=-j1*(n2*ud*dd+na*ud2);
					if(i==0) Hit[i].FricV+=fv*ud;
					else Hit[i].FricV=fv*ud;
					//ApplyImpulse(fv,hitPos);
					fvTotal+=fv;
					hitPosTotal+=hitPos;
					hn2++;
					if(HitN>=1) {
						L-=rmax.cross(n*j1*ud);
					}
				}
			}
			if(hn2>=1)ApplyImpulse(fvTotal,hitPosTotal/(GFloat)hn2); //�͂𔭐��_�̕��ψʒu�œ��͂��Ă�?����������?  //�͂�ړ_���Ŋ����Ă�? �Ȃ��?

		}
		//�߂荞�񂾕���߂�
//		if(Top->TotalCount<=4) dmin=dmin/2;
		if(Top->Type!=GTYPE_BALL) dmin=dmin/2;
		//Bias +=nmin*((-1+dmin)*(-1+dmin)-1);
		Bias -=nmin*dmin;
	}
}

//************************************************
// ���́FP�EL����V�EW�EX�EQ�����߂�
//************************************************
void GRigid::Calc()
{
	if(ChipType==GT_COWL) {
//		R=Q.matrix();
//		TM=R.translate(X);
		return;
	}
	//	if(P.abs()>4000) {
	//		World->Stop=true;
	//		return;
	//	}
	GFloat Dt=World->Dt;
	//���i�^���ʂ���V�������x�����߂�
	preV=V;
	V=P*M_;
	//���~�b�^
	GFloat SPEEDLIMIT=GSPEEDLIMIT;
	if(V.abs()>SPEEDLIMIT) {
		if(Top->ID==0) {
			V=V/V.abs()*SPEEDLIMIT;
			P=V*M;
		}
		else if(V.abs()>SPEEDLIMIT/8*10) {
			V=V/V.abs()*SPEEDLIMIT/8*10;
			P=V*M;
		}
	}
	//��]�^���ʂ���V�����p���x�����߂�
	W=L*I_;
	//���~�b�^

	if(W.abs()*Dt>M_PI/2) { //�^�C���ɂ����169.0f�̐ڒn���x
		W=W/(W.abs()*Dt)*(GFloat)M_PI/2.0f;
		L=W*I;
	}

	//�V�����ʒu�̌v�Z
	X+=(preV+V)*0.5f*Dt;	//���x*dt��������
	//�V�����p���̌v�Z
	Q=(Q+(GQuat(W.x,W.y,W.z,0)*Q/2*Dt)).unit();	//�p���x*Dt��������
	/*
	if(World->Method==0) {//�I�C���[�@
		Q=(Q+(GQuat(W.x,W.y,W.z,0)*Q/2*Dt)).unit();	//�p���x*Dt��������
	}
	else {	//�����Q�E�N�b�^�@
		GQuat	q(Q);
		GMatrix33	r(q.matrix33());	
		GVector	w(L * (r.transpose()*Ib_*r));
		GQuat	k1(GQuat(w.x,w.y,w.z,0) * q * 0.5 * Dt);
		q=Q+k1/2;	r=q.matrix();	w = L * (r.transpose()*Ib_*r);
		GQuat	k2(GQuat(w.x,w.y,w.z,0) * q * 0.5 * Dt);
		q=Q+k2/2;	r=q.matrix();	w = L * (r.transpose()*Ib_*r);
		GQuat	k3(GQuat(w.x,w.y,w.z,0) * q * 0.5 * Dt);
		q=Q+k3;	r=q.matrix();	w = L * (r.transpose()*Ib_*r);
		GQuat	k4(GQuat(w.x,w.y,w.z,0) * q * 0.5 * Dt);
		Q = (Q+(k1 + k2*2 + k3*2 + k4) / 6).unit();
	}
	*/
	Q.matrix(R);
	R.transpose(Rt);
	//�p�����ς��΃e���\�����ς��
	I_ = Rt*Ib_*R;		//OK
	I_.inverse(I);
	
	
	//�V�����ʒu�E�p���p�s��̍쐬
	TM=R.translate(X);
}

//************************************************
// ���́FP�EL����V�EW�EX�EQ�����߂�
//************************************************
void GRigid::CalcLight()
{
	if(ChipType==GT_COWL) return;

	GFloat Dt=World->Dt;
	//���i�^���ʂ���V�������x�����߂�
	preV=V;
	V=P*M_;
	//���~�b�^
	GFloat SPEEDLIMIT=GSPEEDLIMIT;
	if(V.abs()>SPEEDLIMIT) {
		if(Top->ID==0) {
			V=V/V.abs()*SPEEDLIMIT;
			P=V*M;
		}
		else if(V.abs()>SPEEDLIMIT/8*10) {
			V=V/V.abs()*SPEEDLIMIT/8*10;
			P=V*M;
		}
	}
	//��]�^���ʂ���V�����p���x�����߂�
	W=L*I_;

	//�V�����ʒu�̌v�Z
	X+=(preV+V)*0.5f*Dt;	//���x*dt��������
	//�V�����p���̌v�Z
	Q=(Q+(GQuat(W.x,W.y,W.z,0)*Q/2*Dt)).unit();	//�p���x*Dt��������
	Q.matrix(R);
	Rt=R.transpose();
	//�p�����ς��΃e���\�����ς��
	I_ = Rt*Ib_*R;		//OK
	I_.inverse(I);
	
	
	//�V�����ʒu�E�p���p�s��̍쐬
//	TM=R.translate(X);
}
//************************************************
// ���́F���ʍs��̎Z�o
//************************************************
GMatrix33	GRigid::CalcMassMat(GVector &p )
{
	GMatrix33		mm(false);

	GFloat i00=I_.elem[0][0],i01=I_.elem[0][1],i02=I_.elem[0][2];
	GFloat i11=I_.elem[1][1],i12=I_.elem[1][2];
	GFloat i22=I_.elem[2][2];
	
	GFloat xx=p.x*p.x;
	GFloat yy=p.y*p.y;
	GFloat zz=p.z*p.z;
	GFloat xy=p.x*p.y;
	GFloat xz=p.x*p.z;
	GFloat yz=p.y*p.z;

    mm.elem[0][0] = i11*zz - 2*i12*yz + i22*yy + M_;
    mm.elem[1][0] = mm.elem[0][1] = i12*xz - i01*zz - i22*xy + i02*yz;
    mm.elem[2][0] = mm.elem[0][2] = i01*yz - i11*xz - i02*yy + i12*xy;

    mm.elem[1][1] = i22*xx - 2*i02*xz + i00*zz + M_;
    mm.elem[2][1] = mm.elem[1][2] = i02*xy - i12*xx - i00*yz + i01*xz;

    mm.elem[2][2] = i00*yy - 2*i01*xy + i11*xx + M_;


	
	return mm;
}
//***************************************************************
//	���E�N���X���\�b�h
//***************************************************************
GWorld::GWorld(GFloat steptime,int substep)
{
	Alt=false;
	Stop=false;
	NetStop=false;
	B20Bullet=false;
	B26Bullet=false;
	DestroyK=GDESTROY_K;
	DestroyFlag=false;
	Method=0;
	StepTime=steptime;
	SubStep=substep;
	Dt=StepTime/SubStep;
	MainStepCount=0;
	ChipCount=0;
	ObjectCount=0;
	G=GVector(0, GDEFAULT_GRAVITY,0);
	for(int i=0;i<GCHIPMAX;i++) {
		Rigid[i]=NULL;
		RecRigid[i]=NULL;
		RndTable[i]=i;
	}
	for(int i=0;i<GOBJMAX;i++) {
		Object[i]=NULL;
		RecObject[i]=NULL;
	}

//	Object[0]->ApplyImpulse(GVector(5000,0,0),Object[0]->X);
	Land=NULL;
}
GWorld::~GWorld()
{
	DeleteRigids();
	DeleteObjects();
	DeleteLand();
}
void GWorld::InitRndTable()
{
	for(int i=0;i<ChipCount;i++) {
		RndTable[i]=i;
	}
}

void GWorld::SetStepTime(float stepTime)
{
	StepTime=stepTime;
	Dt=StepTime/SubStep;
}
void GWorld::SetSubStep(int substep)
{
	SubStep=substep;
	GDTSTEP=substep;
	Dt=StepTime/SubStep;
}

void GWorld::DeleteLand(){
	if(Land) delete Land;
	if(LandRigid) delete LandRigid;
	Land=NULL;
	LandRigid=NULL;
}

void GWorld::DeleteRigids(){
	for(int i=0;i<ChipCount;i++) {
		if(Rigid[i]) delete Rigid[i];
		if(RecRigid[i]) delete RecRigid[i];
		Rigid[i]=NULL;
		RecRigid[i]=NULL;
		RndTable[i]=i;
	}
	ChipCount=0;
}
void GWorld::DeleteObjects(){
	for(int i=0;i<ObjectCount;i++) {
		if(Object[i]) delete Object[i];
		if(RecObject[i]) delete RecObject[i];
		Object[i]=NULL;
		RecObject[i]=NULL;
	}
	ObjectCount=0;
}
//************************************************
//���E�F���̂̒ǉ�
//************************************************
GRigid* GWorld::AddRigid(int type,bool fix,GFloat x,GFloat y,GFloat z,GFloat ax,GFloat ay,GFloat az)
{
	if(ChipCount>=GCHIPMAX) return NULL;
	GRigid*  r=Rigid[ChipCount++]=new GRigid(type,fix,x,y,z);
	if(r==NULL) return NULL;
	r->ID=ChipCount-1;
	r->World=this;
//	r->Rb.unity();
	r->Rb=GMatrix33().rotateX(ax*(GFloat)M_PI/180.0f).rotateY(ay*(GFloat)M_PI/180.0f).rotateZ(az*(GFloat)M_PI/180.0f);
	r->Ib_=(r->Rb.transpose())*r->Ib_*r->Rb;
	r->Ib=r->Ib_.inverse();
	r->Reset();

	for(int i=0;i<r->Shape.PointN;i++) {
		r->Shape.Point[i]=r->Shape.Point[i]*r->Rb;
	}
	for(int i=0;i<r->CheckShape.PointN;i++) {
		r->CheckShape.Point[i]=r->CheckShape.Point[i]*r->Rb;
	}
	
	return r;
}
//************************************************
//���E�F���̂̍X�V
//************************************************
GRigid* GWorld::UpdateRigid(GRigid *r,int type,bool fix,GFloat x,GFloat y,GFloat z,GFloat ax,GFloat ay,GFloat az)
{
	r->Fixed=fix;
	r->Ghost=0;
	r->Type=type;
	r->Param=GVector(x,y,z);
	r->Reset();
	
	r->Rb=GMatrix33().rotateX(ax*(GFloat)M_PI/180.0f).rotateY(ay*(GFloat)M_PI/180.0f).rotateZ(az*(GFloat)M_PI/180.0f);
	r->Ib_=(r->Rb.transpose())*r->Ib_*r->Rb;
	r->Ib=r->Ib_.inverse();
	
	for(int i=0;i<r->Shape.PointN;i++) {
		r->Shape.Point[i]=r->Shape.Point[i]*r->Rb;
	}
	for(int i=0;i<r->CheckShape.PointN;i++) {
		r->CheckShape.Point[i]=r->CheckShape.Point[i]*r->Rb;
	}
	
	return r;
}
//************************************************
//���E�F�I�u�W�F�N�g�̒ǉ�
//************************************************
GRigid* GWorld::AddObject(int type,bool fix,GFloat x,GFloat y,GFloat z,GFloat density,GFloat ax,GFloat ay,GFloat az)
{
	if(ObjectCount>=GOBJMAX) return NULL;
	GRigid*  r=Object[ObjectCount++]=new GRigid(type,fix,x,y,z);
	r->ChipType=GT_BALLOBJ;
	r->ID=ObjectCount-1;
	r->World=this;
	r->Density=r->Density*density;
	r->Reset();
	r->E=0.4f;
	r->Us=0.7f;
	r->Ud=0.6f;
	r->Cd=0.5f;
	r->X.y=5;
	
	r->Rb=GMatrix33().rotateX(ax*(GFloat)M_PI/180.0f).rotateY(ay*(GFloat)M_PI/180.0f).rotateZ(az*(GFloat)M_PI/180.0f);
	r->Ib_=(r->Rb.transpose())*r->Ib_*r->Rb;
	r->Ib=r->Ib_.inverse();
	r->RSet();
	for(int i=0;i<r->Shape.PointN;i++) {
		r->Shape.Point[i]=r->Shape.Point[i]*r->Rb;
	}
	for(int i=0;i<r->CheckShape.PointN;i++) {
		r->CheckShape.Point[i]=r->CheckShape.Point[i]*r->Rb;
	}
	
	return r;
}
GLand *GWorld::AddLand(int vn,int fn)
{
	Land=new GLand(vn,fn);
	LandRigid=new GRigid(-1,true,500,500,500);
	LandRigid->World=this;
	LandRigid->X=GVector(0,0,0);
	LandRigid->R=GMatrix33();
	LandRigid->E=1.0;
	LandRigid->Us=1.0;
	LandRigid->Ud=1.0;
	return Land;
}
void GWorld::CheckJoint(GRigid* rigidA, GVector &offsetA, GRigid* rigidB, GVector &offsetB)
{
	//�S���_�̃��[���h���W�����߂�
	GVector		s = (offsetA * rigidA->TM - offsetB * rigidB->TM)*Dt*50.0f;
	rigidB->TranslateWithParentAll2(-s);
	rigidB->TranslateWithChild(s);
	rigidB->RSet();
}

void GWorld::CheckShaft(GRigid* rigidA, GVector &offsetA, GRigid* rigidB, GVector &offsetB, GVector &axis)
{
	//�S���_�̃��[���h���W�����߂�
	GVector an=axis*rigidA->R;
	GVector bn=axis*rigidB->R;
	GVector n=an.cross(bn).normalize2();
	GFloat dt2=Dt*50.0f;
	GFloat a=an.angleFast(bn)*dt2;
	GVector		r = (offsetA * rigidA->TM + offsetB * rigidB->TM)/2;
	GVector		s =(offsetA * rigidA->TM - offsetB * rigidB->TM)*dt2;
	if(a!=0) {
		//		rigidA->RotateWithParentAll2(GMatrix().rotate(n,a),r);
		rigidB->RotateWithChild(GMatrix33().rotate(n,-a),r);
	}
	
	//	rigidA->TranslateWithParentAll2(-s);
	rigidB->TranslateWithChild(s);
	rigidB->RSet();
}

void GWorld::DeleteLinkSub(GRigid* rigid,GRigid* top)
{
	//	if(rigid==NULL) return;
	rigid->Top=top;
	for(int i=0;i<rigid->ChildCount;i++) {
		if(rigid->Child[i].RigidB) {
			DeleteLinkSub(rigid->Child[i].RigidB,top);
		}
	}
}
void GWorld::DeleteLink(GRigid* rigid)
{
	if(rigid==NULL) return;
	GRigid *p=rigid->Parent;
	if(p==NULL) return;
	for(int i=0;i<p->ChildCount;i++) {
		if(p->Child[i].RigidB==rigid) {
			DeleteLinkSub(rigid,rigid);
			p->Child[i].RigidB->CheckShape=p->Child[i].RigidB->Shape;
			p->Child[i].RigidB->Parent=NULL;
			p->Child[i].RigidB->LinkInfo=NULL;
			p->Child[i].RigidB->PowerSave=p->Child[i].RigidB->Power;
			p->Child[i].RigidB2=p->Child[i].RigidB;
			p->Child[i].RigidB=NULL;
			break;
		}
	}
	p->CalcTotalFuel();
	rigid->CalcTotalFuel();
}
void GWorld::RestoreLink(GRigid* rigid,GRigid* top)
{
	if(rigid==NULL) return;
	rigid->V.clear();
	rigid->preV.clear();
	rigid->W.clear();
	rigid->P.clear();
	rigid->L.clear();
	rigid->P2.clear();
	rigid->L2.clear();
	rigid->Top=top;
	rigid->TM=rigid->R.translate(rigid->X);
	rigid->RSet();
	rigid->SetTensor();
	rigid->Rt=rigid->R.transpose();
	rigid->Power=rigid->Power2;
	rigid->Energy=0;
	rigid->Fuel=rigid->FuelMax;
	rigid->ByeFlag=0;
	rigid->Tolerant=10000;
	for(int i=0;i<rigid->ChildCount;i++) {
		rigid->CheckShape=rigid->SaveShape;
		if(rigid->Child[i].RigidB==NULL) {
			rigid->Child[i].RigidB=rigid->Child[i].RigidB2;
			rigid->Child[i].RigidB->Parent=rigid;
			rigid->Child[i].RigidB->Top=top;

			rigid->Child[i].RigidB->LinkInfo=&rigid->Child[i];
//			rigid->Child[i].Angle=rigid->Child[i].Angle2;

		}
		rigid->Child[i].RigidB->R=GMatrix33().rotate(rigid->Child[i].Axis,rigid->Child[i].Angle*(GFloat)M_PI/180.0f)*rigid->R;
		rigid->Child[i].RigidB->X=rigid->X+rigid->Child[i].OffsetA*rigid->R-rigid->Child[i].OffsetB*rigid->Child[i].RigidB->R;
		RestoreLink(rigid->Child[i].RigidB,top);
	}
}
GLink *GWorld::AddJoint(GRigid* rigidA, GVector &offsetA, GRigid* rigidB, GVector &offsetB,GFloat friction)
{
	if(rigidB->Parent) return NULL; //���łɍS����������̂��S���ł��Ȃ��B
	if(rigidA->ChildCount>=GCHILDMAX) return NULL;
	rigidB->Parent=rigidA;
	rigidB->Top=rigidA->Top;
	GLink *l=&rigidA->Child[rigidA->ChildCount];
	rigidB->LinkInfo=l;
	l->Type=0;
	l->RigidB=rigidB;
	l->OffsetA=offsetA;
	l->OffsetB=offsetB;
	l->SpringK=1;
	l->DamperK=0.5;
	l->PreDestroyRatio=0;
	l->FrictionK=friction;
	l->DestroyK=DestroyK*(rigidA->Density+rigidB->Density);
	rigidA->ChildCount++;
	rigidB->X=rigidA->X+offsetA-offsetB;
	rigidB->TM=rigidB->R.translate(rigidB->X);
	
	return l;
}
GLink *GWorld::AddShaft(GRigid* rigidA, GVector &offsetA, GRigid* rigidB, GVector &offsetB, GVector &axis,GFloat friction)
{
	if(rigidB->Parent) return NULL; //���łɍS����������̂��S���ł��Ȃ��B
	if(rigidA->ChildCount>=GCHILDMAX) return NULL;
	rigidB->Parent=rigidA;
	rigidB->Top=rigidA->Top;
	GLink *l=&rigidA->Child[rigidA->ChildCount];
	rigidB->LinkInfo=l;
	l->Type=1;
	l->RigidB=rigidB;
	l->OffsetA=offsetA;
	l->OffsetB=offsetB;
	l->Axis=axis.normalize();
	l->SpringK=1;
	l->DamperK=0.5;
	l->PreDestroyRatio=0;
	l->FrictionK=friction;
	l->DestroyK=DestroyK*(rigidA->Density+rigidB->Density);
	rigidA->ChildCount++;
	rigidB->R=rigidA->R;
	rigidB->X=rigidA->X+offsetA*rigidA->R-offsetB*rigidB->R;
	rigidB->RSet();
	return l;
}
GLink *GWorld::AddHinge(GRigid* rigidA, GVector &offsetA, GRigid* rigidB, GVector &offsetB, GVector &axis,GFloat angle,GFloat k,GFloat damper)
{
	//���łɍS����������̂��S���ł��Ȃ��B
	if(rigidB->Parent ||rigidA->ChildCount>=GCHILDMAX) return NULL;
	rigidB->Parent=rigidA;
	rigidB->Top=rigidA->Top;
	GLink *l=&rigidA->Child[rigidA->ChildCount];
	rigidB->LinkInfo=l;
	l->Type=2;
	l->RigidB=rigidB;
	l->OffsetA=offsetA;
	l->OffsetB=offsetB;
	l->Axis=axis.normalize();
	l->Angle=angle;
	l->SpringK=k;
	l->DamperK=damper;
	l->PreDestroyRatio=0;
	l->DestroyK=DestroyK*(rigidA->Density+rigidB->Density);
	rigidB->R=rigidA->R.rotate(axis*rigidA->R,-angle*(GFloat)M_PI/180.0f);
	rigidB->X=rigidA->X+offsetA*rigidA->R-offsetB*rigidB->R;
	rigidB->RSet();
	rigidA->ChildCount++;
	return l;
}
GLink *GWorld::AddCowl(GRigid* rigidA, GVector &offsetA, GRigid* rigidB, GVector &offsetB, GVector &axis,GFloat angle)
{
	//���łɍS����������̂��S���ł��Ȃ��B
	if(rigidB->Parent ||rigidA->ChildCount>=GCHILDMAX) return NULL;
	rigidB->CowlTop=rigidA->CowlTop;
//	if(rigidB->CowlTop==NULL)
//		return NULL;
	rigidB->Parent=rigidA;
	rigidB->Top=rigidA->Top;
	GLink *l=&rigidA->Child[rigidA->ChildCount];
	rigidB->LinkInfo=l;
	l->Type=3;
	l->RigidB=rigidB;
	l->OffsetA=offsetA;
	l->OffsetB=offsetB;
	l->Axis=axis.normalize();
	l->Angle=angle;
	rigidB->R=rigidA->R.rotate(axis*rigidA->R,-angle*(GFloat)M_PI/180.0f);
	rigidB->X=rigidA->X+offsetA*rigidA->R-offsetB*rigidB->R;
	rigidB->RSet();
	rigidA->ChildCount++;
	return l;
}
//************************************************
//���E�F�q�̃����N���`�F�b�N����
//************************************************
void GWorld::CheckLink(GRigid* rigidA)
{
	if(rigidA->ChipType==GT_COWL) return;
//	if(rigidA->Top->TotalMass<800) return;
	GFloat kk=(rigidA->Top->TotalMass-1000)/2000.0f;
	if(kk<0) kk=0;else if(kk>1.0f) kk=1.0;
	GLink *l;
	GVector v;
	int j;
	for(j=0;j<rigidA->ChildCount;j++) {
		l=&rigidA->Child[j];
		if(l) {
			GRigid *rigidB=l->RigidB;
			if(rigidB && rigidB->ChipType!=GT_COWL) {
				GFloat d=l->DamperK;
				if(d<0) d=0;else if(d>1.0) d=1.0;
				GVector ax=l->Axis*rigidA->R;
				GVector bx=l->Axis*rigidB->R;
				GVector n=ax.cross(bx).normalize2();
				if(n.abs()>0.0001) {
					GFloat a=ax.angleFast(bx);
					if(a*d>M_PI/10.0) {
						DeleteLink(rigidB);
						rigidB->ByeFlag=2;
						rigidB->P/=10.0f;
						rigidB->L/=10.0f;
						return;
					}
					if(l->SpringK>=1.0f){
						rigidB->R=rigidB->R.rotate(n,-a*d*kk*10.0f/GDTSTEP);
					}
				}
				v=(rigidA->X+l->OffsetA*rigidA->R-l->OffsetB*rigidB->R-rigidB->X);
				GFloat fv=v.abs();
				if(fv>(CHIPSIZE/2.0f) && l->PreDestroyRatio>(CHIPSIZE/2.0f)) {
					DeleteLink(rigidB);
					rigidB->ByeFlag=2;
					rigidB->P/=10.0f;
					rigidB->L/=10.0f;
					return;
				}
				l->PreDestroyRatio=fv;
				if(l->SpringK>=1.0f){
					rigidB->X+=v*d*kk*10.0f/(GFloat)GDTSTEP;
					rigidB->RSet();
				}
				if(rigidB->MeshNo!=2 && rigidB->MeshNo!=3) CheckLink(rigidB);
			}
		}
	}
}
//************************************************
//���E�F�q�̃����N���v�Z����
//************************************************
void GWorld::CalcLink(GRigid* rigidA)
{
	GLink *l;
	int j,k;
	GFloat f=0;
	for(j=0;j<rigidA->ChildCount;j++) {
		if(Alt) k=j;else k=rigidA->ChildCount-1-j;
		l=&rigidA->Child[k];
		GRigid *rigidB=l->RigidB;
		if(rigidB && rigidB->ChipType!=GT_COWL) {
			CalcLink(rigidB); //switch������̂����肷�� ���U�����N����
			switch(l->Type) {
			case 0:
				f=CalcJoint(rigidA,l->OffsetA,rigidB,l->OffsetB,l->FrictionK)/1.2f;
				break;
			case 1:
				f=CalcShaft(rigidA,l->OffsetA,rigidB,l->OffsetB,l->Axis,l->FrictionK)/2.5f;
				break;
			case 2:
				f=CalcHinge(rigidA,l->OffsetA,rigidB,l->OffsetB,l->Axis,l->Angle,l->SpringK,l->DamperK)/1.2f;
				break;
			}

			rigidB->preF=(rigidB->preF*4+f)/5;
			if(DestroyFlag && l->Type<=2 && ((l->Type==2 && rigidB->MeshNo!=6 && rigidB->preF>l->DestroyK/3.0) || f>l->DestroyK) ) {
				DeleteLink(rigidB);
				rigidB->ByeFlag=2;
				rigidB->P/=10.0f;
				rigidB->L/=10.0f;
			}
		}
	}
}
//************************************************
//���E�F�q�̃����N���v�Z����
//************************************************
void GWorld::CalcLinkCowl(GRigid* rigidA)
{
	GLink *l;
	int j;
	GFloat f=0;
	for(j=0;j<rigidA->ChildCount;j++) {
		l=&rigidA->Child[j];
		GRigid *rigidB=l->RigidB;
		if(rigidB) {
			if(l->Type==3) {
				rigidB->X=rigidA->X+l->OffsetA*rigidA->R-l->OffsetB*rigidB->R;
				rigidB->R=rigidA->R.rotate(l->Axis*rigidA->R,l->Angle*(GFloat)M_PI/180.0f);
				rigidB->RSet();
			}
			CalcLinkCowl(rigidB);
		}
	}
}

//************************************************
//���E�F�W���C���g�i�_�łȂ��j
//************************************************
GFloat GWorld::CalcJoint(GRigid* rigidA, GVector &offsetA, GRigid* rigidB, GVector &offsetB,GFloat friction)
{
	//�S���_�̃��[���h���W�����߂�
	GVector	pA = offsetA * rigidA->R;
	GVector	pB = offsetB * rigidB->R;
	GVector	wA = pA + rigidA->X;
	GVector	wB = pB + rigidB->X;
	GMatrix33	mm = ((rigidA->CalcMassMat(pA) + rigidB->CalcMassMat(pB)).inverse());
	/*
	GVector AV=rigidA->P * rigidA->M_;
	GVector BV=rigidB->P * rigidB->M_;
	GVector AW=rigidA->L * rigidA->I_;
	GVector BW=rigidB->L * rigidB->I_;
	GVector	b = GVector(0,0,0);
	GVector c = (BV-AV)*1.5f+(BW.cross(pB)-AW.cross(pA))*0.5f ;
	*/
	GVector	b = (rigidB->V + pB.cross(rigidB->W)) - (rigidA->V + pA.cross(rigidA->W)); //V��W�͌Â��l������܂����Ȃ���?
	GVector	c  = (rigidB->P * rigidB->M_ + (rigidB->L * rigidB->I_).cross(pB))
		- (rigidA->P * rigidA->M_ + (rigidA->L * rigidA->I_).cross(pA)) ; //P*M_��V����L*I_��W�����A�����Ōv�Z���ׂ�����Ȃ���?
	GVector a=wB - wA;
	GFloat s;
	s=(GFloat)pow((double)a.abs(),(double)1/2.0);
	GVector	F = (s*10.0f*a/Dt+b*0.5f+c)*mm;
	rigidA->ApplyImpulse2(  F ,pA);
	rigidB->ApplyImpulse2( -F ,pB);
	return F.abs();
}
/*
GFloat GWorld::CalcJoint(GRigid* rigidA, GVector &offsetA, GRigid* rigidB, GVector &offsetB,GFloat friction)
{
	//�S���_�̃��[���h���W�����߂�
	GVector	pA = offsetA * rigidA->R;
	GVector	pB = offsetB * rigidB->R;
	GVector	wA = pA + rigidA->X;
	GVector	wB = pB + rigidB->X;
	GVector	b = (rigidB->V + pB.cross(rigidB->W)) - (rigidA->V + pA.cross(rigidA->W));
	GMatrix33	mm = (rigidA->CalcMassMat(pA) + rigidB->CalcMassMat(pB)).inverse();
	GVector	c  = (rigidB->P * rigidB->M_ + (rigidB->L * rigidB->I_).cross(pB))
		- (rigidA->P * rigidA->M_ + (rigidA->L * rigidA->I_).cross(pA)) ;
	GFloat s=(GFloat)sqrt((wB - wA).abs());
	GVector	F = 10*(s*(wB - wA)/Dt+b*0.5f+c)*mm;
	rigidA->ApplyImpulse2(  F ,pA);
	rigidB->ApplyImpulse2( -F ,pB);
	return F.abs();
}
*/
//************************************************
//���E�F�W���C���g�i�_�łȂ��j
//************************************************
GFloat GWorld::CalcJoint2(GRigid* rigidA, GVector &offsetA, GRigid* rigidB, GVector &offsetB,GVector &axis,GFloat friction)
{
	//�S���_�̃��[���h���W�����߂�
	
	rigidB->RSet();
	rigidA->RSet();
	GVector		wA = offsetA * rigidA->TM;
	GVector		wB = offsetB * rigidB->TM;
	GVector		pA = wA - rigidA->X;
	GVector		pB = wB - rigidB->X;
	GVector		a = wB - wA;
	GVector		b = (rigidB->V + pB.cross(rigidB->W) - (rigidA->V + pA.cross(rigidA->W)));
	GMatrix33		mm = (rigidA->CalcMassMat(pA) * rigidB->CalcMassMat(pB))*((rigidA->CalcMassMat(pA) + rigidB->CalcMassMat(pB)).inverse());
	GVector		c  = (rigidB->P * rigidB->M_ + (rigidB->L * rigidB->I_).cross(pB))
		- (rigidA->P * rigidA->M_ + (rigidA->L * rigidA->I_).cross(pA)) ;
	GVector		F = (a/Dt+b*1.0f+c)*mm;
	rigidA->ApplyImpulse(  F ,wA);
	rigidB->ApplyImpulse( -F ,wB);
	GVector ax=axis*rigidA->R;
	GVector bx=axis*rigidB->R;
	GVector n=ax.cross(bx).normalize2();
	if(n.abs()!=0) {
		GFloat angle=ax.angleFast(bx)/Dt/Dt/4.0f;
		GVector t=n*(angle)*rigidB->I+n*(angle)*rigidA->I;
	}
	return F.abs();
}

//************************************************
//���E�F�Ԏ�
//************************************************
GFloat  GWorld::CalcShaft(GRigid* rigidA, GVector &offsetA, GRigid* rigidB, GVector &offsetB, GVector &axis,GFloat friction=0)
{
	GVector ar=axis*rigidA->R;
	GVector absAx=GVector((GFloat)fabs(ar.x),(GFloat)fabs(ar.y),(GFloat)fabs(ar.z));
//	GVector ax=(offsetA.abs()+offsetB.abs())/2.0f*axis;
	GVector D=(rigidA->L*rigidA->I_*absAx-rigidB->L*rigidB->I_*absAx)*absAx;
	GVector Db=(D.normalize2())*friction*Dt;
	if(Db.abs()>D.abs()) Db=D;
	rigidA->L-=Db;
	rigidB->L+=Db;
	//	f  =CalcJoint2( rigidA, offsetA, rigidB, offsetB,axis,0);
	GFloat f=0.0;
		if(myRand()%2) {
			f=CalcJoint( rigidA, offsetA+axis, rigidB, offsetB+axis,0);
			f+=CalcJoint( rigidA, offsetA-axis, rigidB, offsetB-axis,0);
		}
		else {
			f=CalcJoint( rigidA, offsetA-axis, rigidB, offsetB-axis,0);
			f+=CalcJoint( rigidA, offsetA+axis, rigidB, offsetB+axis,0);
		}
	//���~�b�^
	return f;
}
//************************************************
//���E�F�q���W(�΂˕t���Ԏ��j
//************************************************
GFloat GWorld::CalcHinge(GRigid* rigidA, GVector &offsetA, GRigid* rigidB, GVector &offsetB, GVector &axis,GFloat angle=0,GFloat k=1,GFloat damper=0)
{

	GFloat h=10.0f/(GFloat)GLOOP*(GFloat)30.0f/(GFloat)LIMITFPS*10.0f/(GFloat)GDTSTEP;
	k=k*h*0.6f/(GFloat)CHIPSIZE;if(k>1.0f) k=1.0f;else if(k<0) k=0.0f;
	damper=damper*h;if(damper>0.5) damper=0.5f;else if(damper<0) damper=0.0f;
	angle=angle*(GFloat)M_PI/180.0f+(GFloat)M_PI;
	//	rigidA->N2.x=angle;
	GVector ax=axis*rigidA->R;
	GVector bx=axis*rigidB->R;
	GVector oa;
	if(fabs(axis.y)<0.5) oa=VecY.cross(axis);
	else oa=VecX.cross(axis);
	GVector oar=oa*rigidA->R;
	GVector absAx=GVector((GFloat)fabs(ax.x),(GFloat)fabs(ax.y),(GFloat)fabs(ax.z));
	GVector brx=-oa*(GMatrix33().rotate(axis,-angle))*rigidB->R;
	GVector n1= ((oar).cross(ax));
	GFloat a=(oar).angleFast(brx)/((GFloat)M_PI/18.0f);
	if(n1.dot(brx)>=0) a=-a;
	a=a*k*k/2.0f;
	//	else a=a*(GFloat)fabs(a)*k*k/4.0f;
	GVector F=a*((bx*rigidB->I).abs()*rigidA->M+(ax*rigidA->I).abs()*rigidB->M)/(rigidA->M+rigidB->M)/Dt*ax;
	//	F.clear();
	GVector D=(rigidA->L-rigidB->L)*absAx*damper-F;
	rigidA->L-=D;
	rigidB->L+=D;
	GFloat f;
	if(myRand()%2) {
		f=CalcJoint( rigidA, offsetA+axis, rigidB, offsetB+axis,0);
		f+=CalcJoint( rigidA, offsetA-axis, rigidB, offsetB-axis,0);
	}
	else {
		f=CalcJoint( rigidA, offsetA-axis, rigidB, offsetB-axis,0);
		f+=CalcJoint( rigidA, offsetA+axis, rigidB, offsetB+axis,0);
	}
	return f;
}
//************************************************
//���E�F�J�E��
//************************************************
GFloat GWorld::CalcCowl(GRigid* rigidA, GVector &offsetA, GRigid* rigidB, GVector &offsetB, GVector &axis,GFloat angle)
{
	//���łɍS����������̂��S���ł��Ȃ��B
	rigidB->V=rigidA->V;
	rigidB->X=rigidA->X+offsetA*rigidA->R-offsetB*rigidB->R+rigidA->V*Dt/(GFloat)GLOOP;
	rigidB->R=rigidA->R.rotate(axis*rigidA->R,angle*(GFloat)M_PI/180.0f);
	rigidB->RSet();
	return 0.0;
}
//************************************************
//���E�F�Փ˔���
//************************************************
void GWorld::Check(GRigid *rigid){
	rigid->Top->TotalHitCount=0;
	//LandRigid�͒n��
	Land->List1up(rigid->Top->TotalCenter,rigid->Top->TotalRadius); //1�̌n�p�̏Փ˔����ؽ�
	rigid->CollisionCheck(LandRigid);
}
//************************************************
//���E�F�Փ˔���
//************************************************
void GWorld::CheckObject(GRigid *rigid){
	for(int i=0;i<ObjectCount;i++) {
		if(rigid!=Object[i]) rigid->CollisionCheck(Object[i]);
	}
}

//************************************************
// ���E�F�ړ�����
//************************************************
void GWorld::Move(bool initFlag)
{
	int lc=GLOOP;
	int c,i,j,k;
	Dt=StepTime/SubStep;
	if(Stop||NetStop) return;
	for(j=0;j<ChipCount;j++) {
		Rigid[j]->preX=Rigid[j]->X;
		Rigid[j]->HitChip=0;
		Rigid[j]->HitLand=0;
		Rigid[j]->HitObj=0;
		Rigid[j]->HitBullet=0;
		if(Rigid[j]->Parent==NULL) {
			Rigid[j]->TotalFuelMax=0;
			Rigid[j]->TotalFuel=0;
			Rigid[j]->TotalFuel2=0;
			Rigid[j]->CalcTotalFuel();
		}

	}
//	if(ObjectBallFlag) {
		for(j=0;j<ObjectCount;j++) {
			if(Object[j]) {
				Object[j]->preX=Object[j]->X;
				Object[j]->HitChip=0;
				Object[j]->HitLand=0;
				Object[j]->HitObj=0;
				Object[j]->HitBullet=0;
			}
		}
//	}
//	if(ObjectBallFlag) { //�I�u�W�F�N�g�Ƃ̓����蔻��
		for(i=0;i<Bullet->MaxVertex;i++) {
			if(Bullet->Vertex[i].Life>0) {
				for(j=0;j<ObjectCount;j++) {
					GFloat t=Object[j]->X.distanceOnBallAndLine(Object[j]->Radius,Bullet->Vertex[i].Pos,Bullet->Vertex[i].Vec.normalize2());
					if(t>=0 && t<Bullet->Vertex[i].Vec.abs()) {
						Object[j]->HitBullet++;
						Bullet->Vertex[i].Life=-1;
						GVector x=Bullet->Vertex[i].Pos+t*Bullet->Vertex[i].Vec.normalize2();
						Object[j]->ApplyForce(Bullet->Vertex[i].Vec.normalize2()*Bullet->Vertex[i].Power,x);
						Object[j]->ApplyForce((Object[j]->X-x).normalize2()*Bullet->Vertex[i].Power,x);
						GFloat p=(GFloat)pow((double)Bullet->Vertex[i].Power/15000.0,1.0/3.0);
						JetParticle->Add(2,x,GVector(0,0,0),GVector(0,0,0),(0.2f+(myrand()%50/200.0f))*p*0.08f,p,0.04f,GVector(1,1,1),Bullet->Vertex[i].dpnid,true);
					}
				}
			}
		}
//	}
	//�`�b�v�Ƃ̓����蔻��
	GFloat t,t2;
	GRigid *r=NULL;
	if(haveArm && TickCount*30/LIMITFPS>150) {
		for(i=0;i<Bullet->MaxVertex;i++) {
			if(Bullet->Vertex[i].Life>0) {
				r=NULL;
				t2=100000.0f;
				for(j=0;j<ChipCount;j++) {
					if(Bullet->Vertex[i].Rigid!=Rigid[j] && Rigid[j]->ChipType!=9 && (Rigid[j]->ChipType<32 || myrand()%100>=70)) {
						t=Rigid[j]->X.distanceOnBallAndLine(0.3f,Bullet->Vertex[i].Pos,Bullet->Vertex[i].Vec.normalize2());
						if(t>=0 && t<Bullet->Vertex[i].Vec.abs() && t<Bullet->Vertex[i].Dist && t<t2) {
							t2=t;
							r=Rigid[j];
						}
					}
				}
				if(r!=NULL) {
						Bullet->Vertex[i].Life=-1;
						r->HitBullet++;
						GVector x=Bullet->Vertex[i].Pos+t2*Bullet->Vertex[i].Vec.normalize2();
						r->ApplyForce(Bullet->Vertex[i].Vec.normalize2()*Bullet->Vertex[i].Power,x);
						r->ApplyForce((r->X-x).normalize2()*Bullet->Vertex[i].Power,x);
						if(r->ChipType==GT_CHIPH) r->Tolerant-=(Bullet->Vertex[i].Power/10.0f*25.2f/(r->M*4.0f));
						else r->Tolerant-=(Bullet->Vertex[i].Power/10.0f*25.2f/r->M);

						GFloat p=(GFloat)pow((double)Bullet->Vertex[i].Power/15000.0,1.0/3.0);
						if(r->Tolerant<1) {
							if(r->Crush) {
								JetParticle->Add(2,x,GVector(0,0,0),GVector(0,0,0),(0.4f+(myrand()%50/200.0f))*p*0.08f,p,0.04f,GVector(1,1,1),Bullet->Vertex[i].dpnid,true);
								AttackDataDisp("X << Hit ",Bullet->Vertex[i].dpnid,1);
							}
							else  {
								JetParticle->Add(1,x,GVector(0,0,0),GVector(0,0,0),(0.4f+(myrand()%50/200.0f))*p*0.08f,p,0.04f,GVector(1,1,1),Bullet->Vertex[i].dpnid,true);
								AttackDataDisp("X << Crush ",Bullet->Vertex[i].dpnid,1);
							}
							if(r->Parent==NULL) {
								int c=r->ChildCount;
								for(int k=0;k<c;k++) {
									if(r->Child[k].RigidB) {
										r->Child[k].RigidB->ByeFlag=2;
										DeleteLink(r->Child[k].RigidB);
									}
								}
								r->ByeFlag=2;
								r->Tolerant=-1;
							}
							else {
								DeleteLink(r);
								r->ByeFlag=2;
								r->Tolerant=-1;
							}
							r->Crush=true;
						}
						else {
							JetParticle->Add(2,x,GVector(0,0,0),GVector(0,0,0),(0.4f+(myrand()%50/200.0f))*p*0.08f,p,0.04f,GVector(1,1,1),Bullet->Vertex[i].dpnid,true);
							AttackDataDisp("X << Hit ",(DWORD)Bullet->Vertex[i].dpnid,1);
						}
				}
			}
		}
	}
	//Arm�e�ɂ��y��
	for(i=0;i<Bullet->MaxVertex;i++) {
		if(Bullet->Vertex[i].Life>0 && Bullet->Vertex[i].Dist<0) {
			GFloat p=(GFloat)pow((double)Bullet->Vertex[i].Power/5000.0,1.0/5.0);
			JetParticle->Add(Bullet->Vertex[i].Tar,GVector(0,0,0),GVector(0,0,0),p*0.06f,p,0.04f,GVector(0.5,0.48f,0.4f));
			continue;
		}
		GVector v=Bullet->Vertex[i].Pos+Bullet->Vertex[i].Vec;
		if(Bullet->Vertex[i].Life>0 && Bullet->Vertex[i].Pos.y*v.y<0) { //���ʂɓ���or�o��
			GVector x=Bullet->Vertex[i].Pos.pointOnFaceAndLine(GVector(0,1,0),-0.45f,-Bullet->Vertex[i].Vec.normalize());
			GVector w=Bullet->Vertex[i].Vec.normalize()/3.0f;
			if(Bullet->Vertex[i].Pos.y>=0.0f)w.y=-w.y;
			WaterLineParticle->Add(x,w,GVector(0,-0.01f,0),Bullet->Vertex[i].Size/2,2.0f,0.04f,GVector(1,1,1));
		}
	}

	GroundParticle->Move();
	WaterLineParticle->Move();
	JetParticle->Move();
	Bullet->Move();
	for(j=0;j<ChipCount;j++) {
		if(Rigid[j]->ChipType==GT_COWL) continue;
		Rigid[j]->RSet();
		Rigid[j]->MaxImpulse=0.0;
		Rigid[j]->P2.clear();
		Rigid[j]->L2.clear();
		Rigid[j]->Hit[0].FricV.clear();
		if(Rigid[j]->ChipType==GT_ARM) {
			double e=5000.0*30.0/LIMITFPS;
			if(!EfficientFlag) e=Rigid[j]->Top->CheckFuel(e/ARM_EFF)*ARM_EFF;
			Rigid[j]->Energy+=(GFloat)e;
			if(Rigid[j]->Energy>=Rigid[j]->ArmEnergy) {
				Rigid[j]->Energy=Rigid[j]->ArmEnergy;
			}
			else {
				TotalPower+=(GFloat)fabs(e);
				if(!EfficientFlag) {
					Rigid[j]->Top->UseFuel(e/ARM_EFF);
					Rigid[j]->CalcTotalFuel();
				}
			}
		}
	}
//	if(ObjectBallFlag) {
		for(j=0;j<ObjectCount;j++) {
			Object[j]->RSet();
			Object[j]->MaxImpulse=0.0;
			Object[j]->P2.clear();
			Object[j]->L2.clear();
		}
//	}
	//�ړ�����
	if(MainStepCount<=0) {
		//MainStepCount>=1.0/StepTime���ʂ�O��Rigid[j]->TotalRadius+7.0f/StepTime�̋������ړ������Ⴄ����ؔ�������͂�
		//ؽčX�V���̑��x���̋����ړ�������X�V�|�����ق�������
		Land->List3Reset();
		for(j=0;j<ChipCount;j++) {
			if(Rigid[j]->Parent==NULL) {
				Land->List3up(Rigid[j]->TotalCenter+Rigid[j]->V/2,Rigid[j]->TotalRadius+7.0f/StepTime+Rigid[j]->V.abs()/2); //�Œ��޲���͂�߂āA���x�޲�����|����ׂ�  1.0f/StepTime==�����x���e�l ��/StepTime���Ă��������C�����饥������ʂ�̂�StepTime�ɂ�炸���b����
			}
		}
//		if(ObjectBallFlag) {
			for(j=0;j<ObjectCount;j++) {
				if(Object[j]->Parent==NULL) {
					Land->List3up(Object[j]->TotalCenter,Object[j]->TotalRadius+7.0f/StepTime);
				}
			}
//		}
	}
	Land->List2Reset();
	for(j=0;j<ChipCount;j++) {
		Rigid[j]->preX=Rigid[j]->X;
		if(Rigid[j]->Parent==NULL) {
			Land->List2up(Rigid[j]->TotalCenter+Rigid[j]->V/2*StepTime,Rigid[j]->TotalRadius+7.0f+Rigid[j]->V.abs()/2*StepTime);
		}
	}
//	if(ObjectBallFlag) {
		for(j=0;j<ObjectCount;j++) {
			Object[j]->preX=Object[j]->X;
			if(Object[j]->Parent==NULL) {
				Land->List2up(Object[j]->TotalCenter,Object[j]->TotalRadius+7.0f);
			}
		}
//	}
	GFloat dt1=Dt;
	GFloat dt2=StepTime/SubStep/(GFloat)lc;
	GFloat dt3=StepTime/SubStep;
	GVector xmax,x;
	for(i=0;i<SubStep;i++) {
		for(int ii=0;ii<ChipCount/12+2;ii++) {
			j=myRand()%ChipCount;
			k=myRand()%ChipCount;
			int n=RndTable[j];
			RndTable[j]=RndTable[k];
			RndTable[k]=n;
		}
//		if(i%3==0) {
			for(j=0;j<ChipCount;j++) {
				k=RndTable[j];
				if(Rigid[k]->Parent==NULL) {
					Rigid[k]->TranslateWithParentAll(Rigid[k]->TopBias);
					CheckLink(Rigid[k]);
					Rigid[k]->CalcTotalCenter();
				}
			}
			for(j=0;j<ObjectCount;j++) {
				if(Object[j]->Parent==NULL) {
					Object[j]->TranslateWithParentAll(Object[j]->TopBias);
					CheckLink(Object[j]);
					Object[j]->CalcTotalCenter();
				}
			}
//		}
		Alt=!Alt;
		//�ړ�����
		//�d�͌v�Z
//		if(ObjectBallFlag) {
			for(j=0;j<ObjectCount;j++) {//�d��
				k=j;
				if(!Object[k]->Fixed) Object[k]->ApplyImpulse(G*Object[k]->M*Dt,Object[k]->X);
				Object[k]->HitN=0;
			}
//		}
		for(j=0;j<ChipCount;j++) {
			k=RndTable[j];
			Rigid[k]->HitN=0;
			if(Rigid[k]->ChipType==GT_COWL) continue;
			if(!Rigid[k]->Fixed) Rigid[k]->ApplyImpulse(G*Rigid[k]->M*Dt,Rigid[k]->X);
		}
		//�Փ˔���
//		if(ObjectBallFlag) {
			for(j=0;j<ObjectCount;j++) {
				k=j;
				Object[k]->Bias.clear();
				Object[k]->TopBias.clear();
				Check(Object[k]);//�n�ʂƂ̂����蔻��
			}
			for(j=0;j<ObjectCount;j++) {
				k=j;
				for(int ii=k+1;ii<ObjectCount;ii++) {
					if(Object[k]!=Object[ii]) Object[k]->CollisionCheck(Object[ii]);
				}
			}
//		}
		for(j=0;j<ChipCount;j++) {
			k=RndTable[j];
			if(Rigid[k]->ChipType==GT_COWL) continue;
			Rigid[k]->Bias.clear();
			Rigid[k]->TopBias.clear();
			if(Rigid[k]->Parent==NULL) {
				Check(Rigid[k]);
			}
		}
		for(j=0;j<ChipCount;j++) {
			k=RndTable[j];
			if(Rigid[k]->ChipType==GT_COWL) continue;
			CheckObject(Rigid[k]);
		}
		//��C��R�v�Z
//		if(ObjectBallFlag) {
			for(j=0;j<ObjectCount;j++) {
				k=j;
				if(!Object[k]->Fixed) Object[k]->ApplyExtForce();
			}
//		}
		if(!initFlag) {
			for(j=0;j<ChipCount;j++) {
				k=RndTable[j];
				if(Rigid[k]->ChipType==GT_COWL) continue;
				if(!Rigid[k]->Fixed) Rigid[k]->ApplyExtForce();
			}
		}
//		if(ObjectBallFlag) {
			for(j=0;j<ObjectCount;j++) {
				Object[j]->Impulse();
			}
//		}
		for(j=0;j<ChipCount;j++) {
			k=RndTable[j];
			if(Rigid[k]->ChipType==GT_COWL) continue;
			Rigid[k]->Impulse();
		}
		for(j=0;j<ChipCount;j++) {
			k=RndTable[j];
			if(Rigid[k]->HitN>0) {
				//if(Rigid[k]->Bias.abs()>Rigid[k]->Top->TopBias.abs()) Rigid[k]->Top->TopBias=Rigid[k]->Bias;
				if(fabs(Rigid[k]->Top->TopBias.x)<fabs(Rigid[k]->Bias.x)) Rigid[k]->Top->TopBias.x=Rigid[k]->Bias.x;
				if(fabs(Rigid[k]->Top->TopBias.y)<fabs(Rigid[k]->Bias.y)) Rigid[k]->Top->TopBias.y=Rigid[k]->Bias.y;
				if(fabs(Rigid[k]->Top->TopBias.z)<fabs(Rigid[k]->Bias.z)) Rigid[k]->Top->TopBias.z=Rigid[k]->Bias.z;
				Rigid[k]->X=Rigid[k]->X+Rigid[k]->Bias/2;
			}
			//			if(Rigid[j]->Parent==NULL && Rigid[j]->TotalHitCount>0 && !Rigid[j]->Fixed)
			//				Rigid[j]->TranslateWithChild(Rigid[j]->Bias/(GFloat)Rigid[j]->TotalHitCount/4.0f);
		}
		//b=b/ChipCount;
//		if(ObjectBallFlag) {
			for(j=0;j<ObjectCount;j++) {
				k=j;
				if(Object[k]->HitN>0) Object[k]->X=Object[k]->X+(Object[k]->Bias/2);
				//			if(Rigid[j]->Parent==NULL && Rigid[j]->TotalHitCount>0 && !Rigid[j]->Fixed)
				//				Rigid[j]->TranslateWithChild(Rigid[j]->Bias/(GFloat)Rigid[j]->TotalHitCount/4.0f);
			}
//		}
		//�A�����̍S������
//		FILE *fp=fopen("test.dat","w");
		for(c=0;c<lc;c++) {
			Dt=dt3;
			for(j=0;j<ChipCount;j++) {
				k=RndTable[j];
				if(Rigid[k]->Parent==NULL) {
					CalcLink(Rigid[k]);
				}

			}
			Dt=dt2;

			for(j=0;j<ChipCount;j++) {
				k=RndTable[j];
				if(Rigid[k]->ChipType==GT_COWL) continue;
				Rigid[k]->Calc();
			}
			Dt=dt1;
		}
		for(j=0;j<ObjectCount;j++) {//�v�Z
			k=j;
			Object[k]->Calc();
		}
		for(j=0;j<ChipCount;j++) {
			k=RndTable[j];
			if(Rigid[j]->Parent==NULL) {
				CalcLinkCowl(Rigid[j]);
			}

		}
/*
		for(j=0;j<ChipCount;j++) {
			k=RndTable[j];
			if(Rigid[k]->Parent==NULL) {
				Rigid[k]->CalcTotalCenter();
			}
		}
//		if(ObjectBallFlag) {
			for(j=0;j<ObjectCount;j++) {
				if(Object[j]->Parent==NULL) {
					Object[j]->CalcTotalCenter();
				}
			}
//		}
*/
		if(Stop||NetStop) break;
	}

	MainStepCount++;
	if(MainStepCount>=1.0/StepTime) MainStepCount=0;
}
//************************************************
// ���E�F�\������
//************************************************
void GWorld::Disp(BOOL net,BOOL bDrawOpaqueSubsets,BOOL bDrawAlphaSubsets,BOOL bDrawShadow)
{
	int j;
	if(bDrawOpaqueSubsets) for(j=0;j<ObjectCount;j++) Object[j]->DispObject();
	if(net!=TRUE){
		if(bDrawOpaqueSubsets){
			for(j=0;j<ChipCount;j++) {
				if(!(Rigid[j]->ChipType==GT_COWL && (((int)Rigid[j]->Effect)&0xf000))) Rigid[j]->Disp();
			}
		}
		if(bDrawShadow){
			for(j=0;j<ChipCount;j++) Rigid[j]->DispShadow();
			for(j=0;j<ObjectCount;j++) Object[j]->DispShadow();
		}
		if(bDrawAlphaSubsets){
			for(j=0;j<ChipCount;j++) {
				if(Rigid[j]->ChipType==GT_COWL && (((int)Rigid[j]->Effect)&0xf000)) Rigid[j]->Disp();
			}
		}
	}

}
//************************************************
// ���E�F�\������
//************************************************
void GWorld::Disp2() //CCD�J�����p
{
	int j;
	for(j=0;j<ChipCount;j++) if(Rigid[j] && Rigid[j]->Top!=Rigid[0]) Rigid[j]->Disp();
}
//************************************************
// ���E�F�\������
//************************************************
void GWorld::ObjectDisp()
{
	int j;
	for(j=0;j<ObjectCount;j++) if(Object[j]) Object[j]->DispObject();
	//for(j=0;j<ObjectCount;j++) Object[j]->DispShadow();
}
//************************************************
// ���E�FJET�\������
//************************************************
void GWorld::JetDisp()
{
	int j;
	for(j=0;j<ChipCount;j++) if(Rigid[j])Rigid[j]->DispJet();
}
