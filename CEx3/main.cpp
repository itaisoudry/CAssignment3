/*
 * main.c

 *
 *  Created on: Jan 7, 2017
 *      Author: soudry
 */
#include "main_aux.h"

int main() {
	char* imagesPath = NULL;
	char* imagesSuffix = NULL;
	char* imagesPrefix = NULL;
	SPPoint*** RGBHistograms = NULL;
	SPPoint*** SIFTDatabase = NULL;
	int* featuresPerImage;
	int numOfImages, numOfBins, numOfFeatures;
	//This array will contain pointers to be checked-if one of them is null, all of them will be set free from memory
	char** validationArray = (char**) malloc(
	CHAR_ALLOCATION_IN_MAIN * sizeof(char*));
	if (validationArray == NULL) {
		printf(ERROR_GENERAL);
		return 0;
	}

	getImagesPath(&imagesPath);
	validationArray[0] = imagesPath;
	validateCharAllocation(validationArray, 1);

	getImagesPrefix(&imagesPrefix);
	validationArray[1] = imagesPrefix;
	validateCharAllocation(validationArray, 2);

	numOfImages = getNumberOfImages();
	getImagesSuffix(&imagesSuffix);
	validationArray[2] = imagesSuffix;
	validateCharAllocation(validationArray, 3);

	numOfBins = getNumberOfBins();
	numOfFeatures = getNumberOfFeatures();

	featuresPerImage = getHistogramsAndSiftDatabase(&RGBHistograms,
			&SIFTDatabase, imagesPath, imagesSuffix, imagesPrefix, numOfImages,
			numOfBins, numOfFeatures);
	if (featuresPerImage == NULL) {
		destroyInputs(imagesSuffix, imagesPrefix, imagesPath);
		return 0;
	}
	char* query = queryOrTerminate();
	if (query == NULL) {
		destroy(RGBHistograms, SIFTDatabase, imagesSuffix, imagesPrefix,
				imagesPath, validationArray, numOfImages);
		return ERROR;
	}
	//as long as the user didn't terminate the program
	while (query[0] != '#') {
		int nFeatures;
		SPPoint** RGBQuery = spGetRGBHist(query, -1, numOfBins);
		SPPoint** SIFTQuery = spGetSiftDescriptors(query, -1, numOfFeatures,
				&nFeatures);
		if (RGBQuery == NULL || SIFTQuery == NULL) {
			printf(ERROR_ALLOCAT);
			destroy(RGBHistograms, SIFTDatabase, imagesSuffix, imagesPrefix,
					imagesPath, validationArray, numOfImages);
			free(query);
			if (RGBQuery != NULL)
				destroyHistOrSIFT(RGBQuery);
			return ERROR;
		}
		int result = searchUsingGlobalFeatures(RGBQuery, RGBHistograms,
				numOfImages);
		//error occurred inside the function
		if (result == ERROR) {
			destroy(RGBHistograms, SIFTDatabase, imagesSuffix, imagesPrefix,
					imagesPath, validationArray, numOfImages);
			free(query);
			destroyHistOrSIFT(RGBQuery);
			return ERROR;
		}
		result = searchUsingLocalFeatures(SIFTQuery, SIFTDatabase, nFeatures,
				numOfImages, featuresPerImage);
		destroyHistOrSIFT(RGBQuery);
		destroyHistOrSIFT(SIFTQuery);
	}
	destroy(RGBHistograms, SIFTDatabase, imagesSuffix, imagesPrefix, imagesPath,
			validationArray, numOfImages);
	free(query);
	return SUCCESS;
}

