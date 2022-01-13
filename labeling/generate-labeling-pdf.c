/*
Copyright 2013 Gregory Hildstrom

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
This program generates a labeling pdf to be overlayed on top of another
pdf as a watermark/stamp. The output works well with pdftk's multistamp
option. This program requires the output of pdftk's dump command as an
input. This program, with pdftk, is useful for Bates labeling/numbering
pdf documents.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int pages = 2000;
char location = 'r';
char *prefix = NULL;
int prefix_length = 0;
char *format = NULL;
int digits = 0;
int number = 0;
char *font = NULL;
int font_size = 0;
FILE *pdf = NULL;
FILE *dump_data = NULL;

int prompt(void)
{
        prefix = (char *)malloc(256);
        format = (char *)malloc(256);
        font = (char *)malloc(256);
        printf("Enter location (l)eft, (c)enter, (r)ight: ");
        scanf(" %c", &location);
        if (location!='l' && location!='c' && location!='r') {
                printf("error: location must be l, c, or r\n");
                return 1;
        }
        printf("Enter prefix: ");
        scanf("%s", prefix);
        prefix_length = strlen(prefix);
        if (prefix_length < 0 || prefix_length > 40) {
                printf("error: 0 <= prefix length <= 40\n");
                return 1;
        }
        printf("Enter number format/start, like 0000123: ");
        scanf("%s", format);
        digits = strlen(format);
        if (digits <= 0 || digits > 9) {
                printf("error: 0 < digits <= 9\n");
                return 1;
        }
        number = strtol(format, NULL, 10);
        if (number < 0 || number > 1000000000) {
                printf("error: 0 <= number <= 1000000000\n");
                return 1;
        }
        printf("Enter font size: ");
        scanf("%s", font);
        font_size = strtol(font, NULL, 10);
        if (font_size < 1 || font_size > 100) {
                printf("error: 1 <= font size <= 100\n");
                return 1;
        }
        return 0;
}

int find_number_of_pages(void)
{
        char *line = NULL;
        int size = 16;
        int found = 0;
        line = (char *)malloc(512);
        while (fgets(line, size, dump_data)) {
                if (strncmp(line, "NumberOfPages: ", size) == 0) {
                        if (fgets(line, 32, dump_data)) {
                                pages = strtol(line, NULL, 10);
                                printf("notice: determined %d pages from pdftk dump_data\n", pages);
                                found = 1;
                                break;
                        }
                }
        }
        free(line);
        if (found)
                return 0;
        return 1;
}

int output_pdf_header(void)
{
        fprintf(pdf, "%%%%PDF-1.0\n");
        fprintf(pdf, "1 0 obj\n");
        fprintf(pdf, "<<\n");
        fprintf(pdf, "/Type /Catalog\n");
        fprintf(pdf, "/Pages 3 0 R\n");
        fprintf(pdf, ">>\n");
        fprintf(pdf, "endobj\n");
        fprintf(pdf, "\n");
        fprintf(pdf, "2 0 obj\n");
        fprintf(pdf, "<<\n");
        fprintf(pdf, "/Type /Font\n");
        fprintf(pdf, "/Subtype /Type1\n");
        fprintf(pdf, "/BaseFont /Helvetica\n");
        fprintf(pdf, ">>\n");
        fprintf(pdf, "endobj\n");
        if (fprintf(pdf, "\n") < 0)
                return 1;
        return 0;
}

int output_pdf_pages_object(void)
{
        int page_object;
        int objects = pages * 2 + 3;
        fprintf(pdf, "3 0 obj\n");
        fprintf(pdf, "<<\n");
        fprintf(pdf, "/Type /Pages\n");
        fprintf(pdf, "/Count %d\n", pages);
        fprintf(pdf, "/Kids [\n");
        for (page_object=4; page_object <= objects; page_object +=2)
                fprintf(pdf, "%d 0 R\n", page_object);
        fprintf(pdf, "]\n");
        fprintf(pdf, ">>\n");
        fprintf(pdf, "endobj\n");
        if (fprintf(pdf, "\n") < 0)
                return 1;
        return 0;
}

void output_page_object(int page_object, int stream_object)
{
        fprintf(pdf, "%d 0 obj\n", page_object);
        fprintf(pdf, "<<\n");
        fprintf(pdf, "/Type /Page\n");
        fprintf(pdf, "/Parent 3 0 R\n");
        fprintf(pdf, "/Resources << /Font << /F1 2 0 R >>\n");
        fprintf(pdf, ">>\n");
        fprintf(pdf, "/MediaBox [0 0 612 1008]\n");
        fprintf(pdf, "/Contents %d 0 R\n", stream_object);
        fprintf(pdf, ">>\n");
        fprintf(pdf, "endobj\n");
}

void output_stream_object(int stream_object)
{
        int box_width = (prefix_length + digits) * font_size * 2 / 3;
        int box_location = 20;
        if (location == 'c')
                box_location = 310 - (box_width / 2);
        else if (location == 'r')
                box_location = 594 - box_width;
        int text_location = box_location + font_size / 8;
        fprintf(pdf, "%d 0 obj\n", stream_object);
        fprintf(pdf, "<< /Length 5 >>\n");
        fprintf(pdf, "stream\n");
        fprintf(pdf, "/Artifact <</Attached [/Bottom]/Type/Pagination>> BDC 0.753 g\n");
        fprintf(pdf, "%d 20 %d %d re\n", box_location, box_width, font_size + font_size / 4);
        fprintf(pdf, "f*\n");
        fprintf(pdf, "BT\n");
        fprintf(pdf, "/F1 %d Tf\n", font_size);
        fprintf(pdf, "1 0 0 1 %d %d Tm\n", text_location, 20 + font_size / 4);
        fprintf(pdf, "0 g\n");
        fprintf(pdf, "0 G\n");
        fprintf(pdf, "[(%s%0*d)] TJ\n", prefix, digits, number);
        fprintf(pdf, "ET\n");
        fprintf(pdf, "endstream\n");
        fprintf(pdf, "endobj\n");
        fprintf(pdf, "\n");
}

int output_pdf_page_and_stream_objects(void)
{
        int page_object;
        int stream_object;
        int objects = pages * 2 + 3;
        for (page_object = 4; page_object <= objects; page_object +=2) {
                stream_object = page_object +1;
                output_page_object(page_object, stream_object);
                output_stream_object(stream_object);
                number++;
        }
        if (fprintf(pdf, "\n") < 0)
                return 1;
        return 0;
}

int output_pdf_trailer(void)
{
        int objects = pages * 2 + 4;
        fprintf(pdf, "xref\n");
        fprintf(pdf, "0 %d\n", objects);
        fprintf(pdf, "trailer\n");
        fprintf(pdf, "<<\n");
        fprintf(pdf, "/Size %d\n", objects);
        fprintf(pdf, "/Root 1 0 R\n");
        fprintf(pdf, ">>\n");
        fprintf(pdf, "startxref\n");
        if (fprintf(pdf, "%%%%EOF") < 0)
                return 1;
        return 0;
}

int main(int argc, char **argv)
{
        printf("\n\nGenerating a Bates Labeling/Numbering PDF File\n\n");
        if (argc != 3) {
                printf("usage: %s <pdftkDumpDataFile> <outputPDFfile>\n", argv[0]);
                return 1;
        }
        if (prompt())
                return 1;
        dump_data = fopen(argv[1], "r");
        if (!dump_data) {
                printf("error: could not open %s for reading\n", argv[1]);
                return 1;
        }
        if (find_number_of_pages())
                printf("warning: could not determine number of pages from pdftk dump_data, using %d\n", pages);
        fclose(dump_data);
        pdf = fopen(argv[2], "w");
        if (!pdf) {
                printf("error: could not open %s for writing\n", argv[2]);
                return 1;
        }
        if (output_pdf_header()) {
                printf("error: could not write pdf header\n");
                return 1;
        }
        if (output_pdf_pages_object()) {
                printf("error: could not write page object\n");
                return 1;
        }
        if (output_pdf_page_and_stream_objects()) {
                printf("error: could not write page and stream objects\n");
                return 1;
        }
        if (output_pdf_trailer()) {
                printf("error: could not write pdf trailer\n");
                return 1;
        }
        fclose(pdf);
        return 0;
}

