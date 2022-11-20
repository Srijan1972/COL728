int printf(char* str,...);

int selSort(int* c, int size)
{
	int i;
	for (i = 0; i<size; i++)
	{
		int j;
		int min, min_index;
		min = c[i];
		min_index = i;
		for (j = i+1; j < size; j++)
		{
			if (c[j] < min)
			{
				min = c[j];
				min_index = j;
			}
		}

		int temp;
		temp = c[min_index];
		c[min_index] = c[i];
		c[i] = temp;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int a[5];
	a[4] = 0;
	a[3] = 1;
	a[2] = 2;
	a[1] = 3;
	a[0] = 4;
	int sorted;
	sorted = selSort(a, 5);	
	int i;
	for(i = 0;i<5;i++){
		printf("%d ",a[i]);
	}
	printf("\n");
	return 0;
}