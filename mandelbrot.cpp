#include "mandelbrot.hpp"
#include <string.h>

using namespace cv;
using namespace std;


Mandelbrot::Mandelbrot(mpf_t x, mpf_t y, mpf_t w, mpf_t h, int enough) 
	: enough(enough) ,
	divMat(new Mat(im_height*surEchantillonage, im_width*surEchantillonage, CV_32SC1)) ,
	img(new Mat(im_height, im_width, CV_8UC3)) ,
	sEMat(new Mat(im_height, im_width, CV_8UC1))
{
	mpf_init2(pos_x, mpf_get_prec(x));
	mpf_init2(pos_y, mpf_get_prec(y));

	mpf_init2(width, mpf_get_prec(w));
	mpf_init2(height, mpf_get_prec(h));

	mpf_init2(atomic_w, mpf_get_prec(w) + ceil(log(im_width*surEchantillonage) / log(2)));
	mpf_init2(atomic_h, mpf_get_prec(h) + ceil(log(im_height*surEchantillonage) / log(2)));

	mpf_set(pos_x, x);
	mpf_set(pos_y, y);
	mpf_set(width, w);
	mpf_set(height, h);
	
	ThresholdCont = 181.75*pow(im_width*im_height,-0.309);
	ThresholdSave = 93.346*pow(im_width*im_height,-0.217);


	mpf_div_ui(atomic_w, width, im_width*surEchantillonage);
	mpf_div_ui(atomic_h, height, im_height*surEchantillonage);
        
        IterUp();
}

Mandelbrot::Mandelbrot(char* buf)
    :divMat(new Mat(im_height*surEchantillonage, im_width*surEchantillonage, CV_32SC1)) ,
    img(new Mat(im_height, im_width, CV_8UC3)) ,
    sEMat(new Mat(im_height, im_width, CV_8UC1))
{
    int prec;
    char* tmp;


    tmp = strtok( buf, ":");
    enough = atoi(tmp);


    tmp = strtok(NULL, ":");
    prec = ceil(strlen(tmp)*log(10)/log(2));
    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
    prec = (prec < 64)?64:prec;

    mpf_init2( pos_x, prec);
    mpf_set_str( pos_x, tmp, 10);


    tmp = strtok(NULL, ":");
    prec = ceil(strlen(tmp)*log(10)/log(2));
    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
    prec = (prec < 64)?64:prec;

    mpf_init2( pos_y, prec);
    mpf_set_str( pos_y, tmp, 10);


    tmp = strtok(NULL, ":");
    prec = ceil(strlen(tmp)*log(10)/log(2));
    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
    prec = (prec < 64)?64:prec;

    mpf_init2( width, prec);
    mpf_set_str( width, tmp, 10);


    tmp = strtok(NULL, ":");
    prec = ceil(strlen(tmp)*log(10)/log(2));
    prec = (prec%64 != 0)?(prec/64)*64+64:(prec/64)*64;
    prec = (prec < 64)?64:prec;

    mpf_init2( height, prec);
    mpf_set_str( height, tmp, 10);


    mpf_init2(atomic_w, mpf_get_prec(width) + ceil(log(im_width*surEchantillonage) / log(2)));
    mpf_init2(atomic_h, mpf_get_prec(height) + ceil(log(im_height*surEchantillonage) / log(2)));

   	
    ThresholdCont = 181.75*pow(im_width*im_height,-0.309);
    ThresholdSave = 93.346*pow(im_width*im_height,-0.217);
	

    mpf_div_ui(atomic_w, width, im_width*surEchantillonage);
    mpf_div_ui(atomic_h, height, im_height*surEchantillonage);
        
    IterUp();
}

Mandelbrot::~Mandelbrot()
{
	if(divMat != nullptr)
	{
		mpf_clears(pos_x, pos_y, width, height, atomic_w, atomic_h, NULL);
		
		delete divMat;
		delete img;
		delete sEMat;

		divMat = nullptr;
	        img = nullptr;
		sEMat = nullptr;
	}
}

void Mandelbrot::del_mem()
{
	mpf_clears(pos_x, pos_y, width, height, atomic_w, atomic_h, NULL);
	
	delete divMat;
	delete img;
	delete sEMat;

	divMat = nullptr;
	img = nullptr;
	sEMat = nullptr;
}

void Mandelbrot::escapeSpeedCalcSeq()
{

    // initialisation des coordonnées
    mpf_t tmp1, tmp2;
    mpf_t *x, *y;
    x = new mpf_t[this->im_width*this->surEchantillonage];
    y = new mpf_t[this->im_height*this->surEchantillonage];

    if(!x || !y)
	exit(2);

    mpf_inits( tmp1, tmp2, NULL);

    mpf_div_ui(tmp1, this->width, 2); //  tmp1 = width/2
    mpf_set_ui( tmp2, 0);
    for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
    {
	mpf_init(x[i]);

	//  xc = pos_x - width/2 + i*atomic_w
	mpf_sub(x[i], this->pos_x, tmp1); //  xc = pos_x - tmp = pos_x - width/2
	//mpf_mul_ui(tmp, this->atomic_w, i); //  tmp = atomic_w * i
	mpf_add( tmp2, tmp2, atomic_w);
	mpf_add(x[i], x[i], tmp2); //  xc = xc + tmp = pos_x - width/2 + atomic_w * i

    }

    mpf_div_ui(tmp1, this->height, 2); //  tmp1 = height/2
    mpf_set_ui( tmp2, 0);
    for(int i = 0; i < this->im_height*this->surEchantillonage; ++i)
    {
	mpf_init(y[i]);

	//  yc = pos_y - height/2 + i*atomic_h
	mpf_sub(y[i], this->pos_y, tmp1); //  yc = pos_y - tmp = pos_y - height/2
	//mpf_mul_ui(tmp, atomic_h, j); //  tmp = atomic_h * j
	mpf_add( tmp2, tmp2, atomic_h);
	mpf_add(y[i], y[i], tmp2); //  yc = yc + tmp = pos_y - height/2 + atomic_h * j
    }

    mpf_clears( tmp1, tmp2, NULL);






    *(this->sEMat) = 1;
    *(this->divMat) = -1;
    

    // calcul de l image
    calcSeq( x, y);	
	
	
	
    // on detecte les zones nécessitant dur sur echantillonage
    this->draw();
    Mat kernel = Mat::ones( 7, 7, CV_8UC1 );

    int lowThreshold = 10;
    int ratio = 3;
    int kernel_size = 3;

    cvtColor( *(this->img), *(this->sEMat), CV_BGR2GRAY );
    blur( *(this->sEMat), *(this->sEMat), Size(3,3) );
    Canny( *(this->sEMat), *(this->sEMat), lowThreshold, lowThreshold*ratio, kernel_size);
    filter2D( *(this->sEMat), *(this->sEMat), -1 , kernel, Point( -1, -1 ), 0, BORDER_DEFAULT);

    *(this->sEMat) = *(this->sEMat)*this->surEchantillonage/255;

    // on recalcul l image avec le sur echantillonage
    calcSeq( x, y);

    // on libere tout
    for(int i = 0; i < this->im_width*this->surEchantillonage; ++i)
    {
	mpf_clear(x[i]);
    }
    for(int i = 0; i < this->im_height*this->surEchantillonage; ++i)
    {
	mpf_clear(y[i]);
    }

    delete [] x;
    delete [] y;
}


void Mandelbrot::calcSeq(mpf_t* x, mpf_t* y)
{
    mpf_t xn, yn, xnp1, ynp1, mod, xsqr, ysqr, tmp;
    mpf_inits( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);

    for(int j = 0; j < im_height; j++)
    {
	for (int i = 0; i < im_width; i++)
	{
	    int sE = sEMat->at<char>( j, i);

	    for(int m = 0; m < sE; m++)
	    {
	        for(int n = 0; n < sE; n++)
		{
		    if((sE != 1 && divMat->at<int>(j*surEchantillonage+n ,i*surEchantillonage+m) == -1) || sE == 1)
		    {
			mpf_set_ui(xn,0);
			mpf_set_ui(yn,0);
			mpf_set_ui(xsqr,0);
			mpf_set_ui(ysqr,0);

			for (int k = 1; k < iterations; k++)
			{
			    //  xnp1 = xn² - yn² + xc
			    mpf_sub(xnp1, xsqr, ysqr); //  xnp1 = xsqr - ysqr = xn² - yn²
			    //mpf_add(xnp1, xnp1, xc); //  xnp1 = xnp1 + xc = xn² - yn² + xc
			    mpf_add(xnp1, xnp1, x[i*surEchantillonage+m]); //  xnp1 = xnp1 + xc = xn² - yn² + xc

			    //  ynp1 = 2*xn*yn + yc
			    mpf_mul(ynp1, xn, yn); //  ynp1 = xn * yn
			    mpf_mul_ui(ynp1, ynp1, 2); //  ynp1 = ynp1 * 2 = 2 * xn * yn
			    //mpf_add(ynp1, ynp1, yc); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc
			    mpf_add(ynp1, ynp1, y[j*surEchantillonage+n]); //  ynp1 = ynp1 + yc = 2 * xn * yn + yc

			    //  xn = xnp1
			    //  yn = ynp1
			    mpf_set( xn, xnp1); //  xn = xnp1
			    mpf_set( yn, ynp1); //  yn = ynp1
							
			    //xsqr = xn²
			    mpf_mul(xsqr, xn, xn);

			    //ysqr = yn²
			    mpf_mul(ysqr, yn, yn);

			    //  mod = xnp1² + ynp1²
			    mpf_add(mod, xsqr, ysqr); //  mod = xsqr + ysqr = xn² + yn²

			    if(mpf_cmp_ui(mod, 4) > 0)
			    {
				divMat->at<int>(j*surEchantillonage+n, i*surEchantillonage+m) = k;
				break;
			    } else if(k == iterations -1)
			    {
			        divMat->at<int>(j*surEchantillonage+n, i*surEchantillonage+m) = iterations;
			    }
			}
		    }
	        }
	    }
	}
    }

    mpf_clears( xn, yn, xnp1, ynp1, mod, tmp, xsqr, ysqr, NULL);

}


void Mandelbrot::draw()
{
	int moy, nbr_div, nbr_ndiv, divSpeed;

	for(int i = 0; i < im_width; ++i)
	{
		for (int j = 0; j < im_height; ++j)
		{
			moy = 0, nbr_div = 0, nbr_ndiv = 0;
			for(int k = 0; k < surEchantillonage; ++k)
			{
				for(int l = 0; l < surEchantillonage; ++l)
				{
					divSpeed = divMat->at<int>( j*surEchantillonage + l, i*surEchantillonage + k);

					if(divSpeed != -1)
					{
						if(divSpeed == iterations)
							nbr_ndiv++;
						else
						{
							moy += divSpeed;
							nbr_div++;
						}
					}
				}
			}

			Vec3b bgr;

			if(nbr_div)
				moy /= nbr_div;
			else 
				moy = iterations;

			switch(color)
			{
				case 1:
				{
					coloration(bgr, moy, iterations, nbr_div, nbr_ndiv);
					break;
				}
				case 2:
				{
					coloration2(bgr, moy, iterations);
					break;
				}
				case 3:
				{
					coloration3(bgr, moy, iterations);
					break;
				}
			}
			img->at<Vec3b>( j, i) = bgr;
		}
	}
}

void Mandelbrot::save()
{
    int nume=matSave( img, Mandelbrot::rep);

    char nom_inf[128];
    sprintf( nom_inf, "../img/%s/info.txt", Mandelbrot::rep);

    FILE* fichier = fopen(nom_inf, "a");

    fprintf(fichier,"mandel%d",nume);
    fprintf(fichier,"\n\tXposition=");
    mpf_out_str(fichier, 10, 150, pos_x);
    fprintf(fichier,"\n\tYposition=");
    mpf_out_str(fichier, 10, 150, pos_y);
    fprintf(fichier,"\n\twidth=");
    mpf_out_str(fichier, 10, 150, width);
    fprintf(fichier,"\n\theight=");
    mpf_out_str(fichier, 10, 150, height);
    fprintf(fichier,"\n");

    

	/*if(mpf_cmp_ui(this->pos_y, 0) != 0)
	{
		flip( *(this->img), *(this->img), 0);
		nume = matSave( this->img, this->rep);

		fprintf(fichier,"mandel%d",nume);
	    fprintf(fichier,"\n\tx : ");
	    mpf_out_str(fichier, 10, 150, pos_x);
	    fprintf(fichier,"\n\ty : ");
	    mpf_out_str(fichier, 10, 150, pos_y);
	    fprintf(fichier,"\n\tw : ");
	    mpf_out_str(fichier, 10, 150, width);
	    fprintf(fichier,"\n\th : ");
	    mpf_out_str(fichier, 10, 150, height);
	    fprintf(fichier,"\n");
	}*/

	fclose(fichier);
}

bool Mandelbrot::IsGood_2(bool* filtre)
{
	*filtre = false;
	bool continue_y_or_n;

	Mat* src_gray = new Mat(im_height, im_width, CV_8UC3);		//entier non signé 8 bit à 3 dimension
	Mat* detected_edges = new Mat(im_height, im_width, CV_8UC1);	//pareil a 2 dimension
	if(!detected_edges) cout<<"fuck"<<endl;
	if(!src_gray) cout<<"fuck2"<<endl;


	int lowThreshold = 30;	
	int ratio = 3;			
	int kernel_size = 3;	

	cvtColor( *(img), *(src_gray), CV_BGR2GRAY );
	
	blur( *(src_gray), *(detected_edges), Size(3,3) );
	Canny( *(detected_edges), *(detected_edges), lowThreshold, lowThreshold*ratio, kernel_size);
	double res = countNonZero(*detected_edges)*1000/(im_height*im_width);

	if(res<ThresholdCont)
		continue_y_or_n = false;
	else
		continue_y_or_n = true;


	if(continue_y_or_n)
	{
		int y0 = im_width/2;
		int x0 = im_height/2;
		int sigma_x = x0/2;
		int sigma_y = y0/2;

		res = 0.0;

		for (int i = 0; i < im_height; i++)
		{
			for (int j = 0; j < im_width; j++)
			{

				double X =(pow(i - x0, 2)/(2*pow(sigma_x, 2)));

				double Y =(pow(j - y0, 2)/(2*pow(sigma_y, 2)));

				double flou = exp(-(X + Y));

				res += (((detected_edges->at<char>(i, j)+256)%256) * flou);
			}
		}
		res = res/(im_height*im_width)*1000/255;

		if(res >= ThresholdSave)
			*filtre = true;
		else
			*filtre = false;
	}

	delete src_gray;
	delete detected_edges;

	return continue_y_or_n;
}


void Mandelbrot::IterUp(){

	//augmentation du nombre d'iteration en fonction de la profondeur du zoom actuel
	//66.5*racine(2*racine(abs(1 - racine(5*(scale)))))	with scale = this->width/3

	mpf_t temp;
	mpf_inits(temp, NULL);

	//temp = scale = this->width/3
	mpf_div(temp, atomic_w, width);

	//temp = 5*temp
	mpf_mul_ui(temp, temp, 5);

	//temp = racine(temp)
	mpf_sqrt(temp, temp);

	//temp = 1 - temp
	mpf_ui_sub(temp, 1, temp);

	//temp = abs(temp)
	mpf_abs(temp, temp);
	
	//temp = racine(temp)
	mpf_sqrt(temp, temp);

	//temp = 2.temp
	mpf_mul_ui(temp, temp, 2);

	//temp = racine(temp)
	mpf_sqrt(temp, temp);

	//temp = temp*66.5
	mpf_mul_ui(temp, temp, 110);

	//convertissation
	double x = mpf_get_d(temp);

	iterations = x;

	mpf_clears(temp, NULL);
	
}

void Mandelbrot::dichotomie3(int n_div, vector<int>& divs)
{
    int prec = mpf_get_prec(pos_x);

    escapeSpeedCalcSeq();
    draw();

    bool filtre;

    if(IsGood_2(&filtre))
    {
	if(filtre)
	    save();

	if(enough)
	{	
	    mpf_t old_pos_x, old_pos_y, old_width, old_height;

	    mpf_init2(old_pos_x, mpf_get_prec(pos_x));
	    mpf_init2(old_pos_y, mpf_get_prec(pos_y));
	    mpf_init2(old_width, mpf_get_prec(width));
	    mpf_init2(old_height, mpf_get_prec(height));

	    mpf_set(old_pos_x, pos_x);
	    mpf_set(old_pos_y, pos_y);
	    mpf_set(old_width, width);
	    mpf_set(old_height, height);

	    // recuper dans les old les valeurs de "this->"
	    // et remplacer les this-> dans la suite par des old_
	    //delete l'image ou on est
	    del_mem();

	    for(int i = 0; i < n_div; i++)
	    {
		int n_prec = prec + ceil(log(divs.at(i))/log(2));
		mpf_t temp, delta_x, delta_y;

		mpf_t* tab_x = new mpf_t[divs.at(i)];
		mpf_t* tab_y = new mpf_t[divs.at(i)];

		if(mpf_get_prec(old_pos_x)>=mpf_get_prec(old_pos_y))
		    mpf_init2(temp, mpf_get_prec(old_pos_x) + n_prec/64);
		else
		    mpf_init2(temp, mpf_get_prec(old_pos_y) + n_prec/64);

		mpf_init2(delta_x, mpf_get_prec(old_width));
		mpf_init2(delta_y, mpf_get_prec(old_height));

		//initialise chacun des elements du tableau avant de pouvoir s'en servir
		for (int init = 0; init < divs.at(i); init++)
		{
			mpf_init2(tab_x[init], mpf_get_prec(old_pos_x) + n_prec/64);
			mpf_init2(tab_y[init], mpf_get_prec(old_pos_y) + n_prec/64);
		}

		n_prec %= 64;


		//calcul de delta_x, la distance entre de nouveaux points en x
		mpf_div_ui(delta_x, old_width, divs.at(i));
		//calcul de delta_y, la distance entre de nouveaux points en y
		mpf_div_ui(delta_y, old_height, divs.at(i));


		//tab_x[0] = pos_x - width/2 + width/2*divs.at(i)
		mpf_div_ui(temp, old_width, 2*divs.at(i));
		mpf_add(tab_x[0], old_pos_x, temp);
		mpf_div_ui(temp, old_width, 2);
		mpf_sub(tab_x[0], tab_x[0], temp);

		//tab_y[0] = pos_y - height/2 + height/2*divs.at(i)
		mpf_div_ui(temp, old_height, 2*divs.at(i));
		mpf_add(tab_y[0], old_pos_y, temp);
		mpf_div_ui(temp, old_height, 2);
		mpf_sub(tab_y[0], tab_y[0], temp);

		for (int c = 1; c < divs.at(i); c++)
		{
		    //tab_x[c] = tab_x[0] + c*delta_x
		    mpf_mul_ui(temp, delta_x, c);
		    mpf_add(tab_x[c], tab_x[0], temp);

		    //tab_y[c] = tab_y[0] + c*delta_y
		    mpf_mul_ui(temp, delta_y, c);
		    mpf_add(tab_y[c], tab_y[0], temp);
		}

		for (int x = 0; x < divs.at(i); x++)
	        {
		    for (int y = 0; y < divs.at(i); y++)
		    {
			if(mpf_cmp_ui(tab_y[y], 0) < 0)
			{
			    Mandelbrot* M = new Mandelbrot(tab_x[x], tab_y[y], delta_x, delta_y , enough - 1);
                            //en bas a gauche
						
			    M->dichotomie3(n_div, divs);
						
			    delete M;
						
			}
		    }
		}

		//delete chacun des element du tableau avant pour pouvoir virer le tableau
		for (int del = 0; del < divs.at(i); del++)
		{
		    mpf_clear(tab_x[del]);
		    mpf_clear(tab_y[del]);
		}

		delete [] tab_x;
		delete [] tab_y;
		mpf_clears(temp, delta_x, delta_y, NULL);
	    }
        }
    }
}

/*
bool Mandelbrot::random_img (int enough, double zoom, gmp_randstate_t& state)
{    
    this->escapeSpeedCalcThread4();
    this->draw();
    
    bool filtre;
    this->IsGood_2(&filtre);

    if(filtre)
    {
        this->save();

    
        int max_test = 10;
        bool good = false;

        if(--enough)
        {
			this->IterUp();
			this->iterations *= 2;

			mpf_t nx, ny ,cx, cy, nw, nh, temp, zoom_mpf, Wdiff, Wdiff2, Hdiff, Hdiff2;

			mpf_inits(nx, ny ,cx, cy, nw, nh, temp, zoom_mpf, Wdiff, Wdiff2, Hdiff, Hdiff2, NULL);

			mpf_set_d(zoom_mpf, zoom);
						
			mpf_div(nw, this->width, zoom_mpf);        //calcul nouveaux W 
			mpf_div(nh, this->height, zoom_mpf);        //calcul nouveaux H
			
			mpf_set(cx,this->pos_x);
			mpf_set(cy,this->pos_y);
			
			
			mpf_sub(Wdiff, this->width, nw);
			
			mpf_neg(Wdiff2, Wdiff);
			mpf_div_ui(Wdiff2, Wdiff2, 2);
			mpf_add(Wdiff2, Wdiff2, cx);
			
			
			mpf_sub(Hdiff, this->height, nh);
			
			mpf_neg(Hdiff2, Hdiff);
			mpf_div_ui(Hdiff2, Hdiff2, 2);
			mpf_add(Hdiff2, Hdiff2, cy);
			
			
			del_mem();
            
            do{
				mpf_urandomb (nx, state, mpf_get_prec(nx));
				mpf_mul(nx, nx, Wdiff);
				mpf_add(nx, nx, Wdiff2);

				mpf_urandomb (ny, state, mpf_get_prec(ny));
				mpf_mul(ny, ny, Hdiff);
				mpf_add(ny, ny, Hdiff2);

				Mandelbrot* M = new Mandelbrot(nx, ny, nw, nh ,im_width, im_height, surEchantillonage, iterations, color, mpmc, rep);

				good = M->random_img(enough, zoom, state);
				delete M;    
            } while( (good == false) && (--max_test > 0) );
            
            mpf_clears(nx, ny ,cx, cy, nw, nh, temp, zoom_mpf, Wdiff, Wdiff2, Hdiff, Hdiff2, NULL);
            if (!max_test) return false;
        }

        return true;
    }
    else{
        return false;
    }   
}



*/
